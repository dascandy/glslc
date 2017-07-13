#ifndef GLSL_H
#define GLSL_H

#include <string>
#include <vector>
#include <map>
#include <string.h>

class AsmBlock;
enum NodeType {
  NFunctionCall,
  NConstantInt,
  NConstantUInt,
  NConstantFloat,
  NExpression,
  NScopeStmt,
  NForLoop,
  NWhileLoop,
  NIfStmt,
  NVarRef,
  NShuffle,
  NReturn,
  NDiscard,
  NBoolChoice,
  NConditionalJump,
  NConstLoad,
  NVarLoad,
  NVarStore,
};

struct DefLocation {
  std::string fileName;
  size_t offs, lineno;
};

void CompileError(DefLocation *loc, const char *err, ...);

struct Type {
  enum _Type {
    Tempty,
    Tpoison,
    Tvoid,
    Tbool,
    Tfloat,
    Tdouble,
    Tint,
    Tuint,
    Tsampler,
    Tisampler,
    Tusampler,
  } type;
  static Type getType(DefLocation loc, std::string typeName);
  Type(_Type t, int sx, int sy, bool constness) {
    type = t;
    sizeX = sx;
    sizeY = sy;
    this->constness = constness;
  }
  static Type Poison, Empty, Void;
  bool operator!=(const Type &t) const {
    return !(*this == t);
  }
  bool operator==(const Type &t) const {
    return type == t.type &&
           sizeX == t.sizeX &&
           sizeY == t.sizeY;
  }
  bool canConvertTo(const Type &t) const {
    return *this == t;
  }
  std::string asString() const;
  size_t sizeX, sizeY;
  DefLocation def;
  bool constness;
};

class Variable {
public:
  Variable(const Type &type, const std::string &name, int location)
    : type(type)
    , name(name)
    , location(location)
    , arraySize(-1) {
  }
  Variable(DefLocation def, const std::string &type, const std::string &name, int location)
    : type(Type::getType(def, type))
    , name(name)
    , location(location)
    , arraySize(-1) {
  }
  void setArray(int size) {
    arraySize = size;
  }
  Type type;
  std::string name;
  int location;
  int arraySize;
  DefLocation def;
};

const char *spaces(int n);

class Rvalue {
public:
  Rvalue(NodeType nt, Type type, DefLocation def)
    : nt(nt)
    , type(type)
    , def(def) {
  }
  virtual void Print(int indent = 0) = 0;
  NodeType nt;
  Type type;
  DefLocation def;
  virtual ~Rvalue() {}
};

class VarRef : public Rvalue {
public:
  VarRef(DefLocation loc, Variable *var)
    : Rvalue(NVarRef, var->type, loc)
    , var(var) {
  }
  void Print(int indent) {
    printf("%s", var->name.c_str());
  }
  Variable *var;
};

class FuncArg {
public:
  FuncArg(DefLocation loc, const std::string &direction, const std::string &type, const std::string &name, Rvalue *initial)
    : direction(direction)
    , type(Type::getType(loc, type))
    , name(name)
    , initial(initial) {
  }
  FuncArg(DefLocation loc, const std::string &direction, const Type &type, const std::string &name, Rvalue *initial)
    : direction(direction)
    , type(type)
    , name(name)
    , initial(initial) {
  }
  Type type;
  std::string direction, name;
  Rvalue *initial;
};

class Expression : public Rvalue {
public:
  enum operation {
    arrayidx,
    assign,
    booland,
    boolor,
    binand,
    binandnot,
    binor,
    binxor,
    comma,
    comp_eq,
    comp_ge,
    comp_gt,
    comp_le,
    comp_lt,
    comp_ne,
    divide,
    invert,
    max,
    min,
    minus,
    neg,
    plus,
    postdecr,
    postincr,
    predecr,
    preincr,
    times
  };
  const char *getName(operation op) {
    const char *names[] = {
      "[]",
      "=",
      "&&",
      "||",
      "&",
      "&~",
      "|",
      "^",
      ",",
      "==",
      ">=",
      ">",
      "<=",
      "<",
      "!=",
      "/",
      "!",
      "max",
      "min",
      "-",
      "-",
      "+",
      "?--",
      "?++",
      "--",
      "++",
      "*"
    };
    return names[op];
  }
  Expression(operation op, Rvalue *left, Rvalue *right = NULL)
    : Rvalue(NExpression, GetType(left, op, right), left->def)
    , left(left)
    , op(op)
    , right(right) {
  }
  void Print(int indent) {
    if (right) {
      left->Print(indent + 1);
      printf(" %s ", getName(op));
      right->Print(indent + 1);
    } else {
      printf("%s", getName(op));
      left->Print(indent + 1);
    }
  }
  Rvalue *left, *right;
  operation op;
  static Type GetType(Rvalue *left, operation op, Rvalue *right);
};

class ImmediateFloatExpression : public Rvalue {
public:
  ImmediateFloatExpression(DefLocation loc, const std::string &imm)
    : Rvalue(NConstantFloat, Type(Type::Tfloat, 1, 1, true), loc)
    , precise(false) {
    SetValue(imm.c_str());
  }
  void SetValue(const char *val);
  void Print(int indent) {
    printf("%f", val);
  }
  float val;
  bool precise;
};

class ImmediateIntExpression : public Rvalue {
public:
  ImmediateIntExpression(DefLocation loc, const std::string &imm)
    : Rvalue(NConstantInt, Type(Type::Tint, 1, 1, true), loc) {
    SetValue(imm.c_str());
  }
  void SetValue(const char *val);
  void Print(int indent) {
    printf("%d", val);
  }
  int val;
};

class ImmediateUintExpression : public Rvalue {
public:
  ImmediateUintExpression(DefLocation loc, const std::string &imm)
    : Rvalue(NConstantUInt, Type(Type::Tuint, 1, 1, true), loc) {
    SetValue(imm.c_str());
  }
  void SetValue(const char *val);
  void Print(int indent) {
    printf("%u", val);
  }
  unsigned int val;
};

class ConstVal {
public:
  ConstVal(const ImmediateFloatExpression &v) {
    memset(value, 0, 16);
    memcpy(value, &v.val, sizeof(v.val));
  }
  ConstVal(const ImmediateIntExpression &v) {
    memset(value, 0, 16);
    memcpy(value, &v.val, sizeof(v.val));
  }
  ConstVal(const ImmediateUintExpression &v) {
    memset(value, 0, 16);
    memcpy(value, &v.val, sizeof(v.val));
  }
  bool operator==(const ConstVal &val) {
    return memcmp(value, val.value, 16) == 0;
  }
  char value[16];
};

class ShuffleExpression : public Rvalue {
public:
  ShuffleExpression(Rvalue *left, const std::string &shuffle)
    : Rvalue(NShuffle, left->type, left->def)
    , left(left)
    , shuffle(shuffle) {
    if (left->type.sizeY != 1) {
      CompileError(&left->def, "Cannot use shuffle operator on non-vector type\n");
      type = Type::Poison;
    } else if (shuffle.size() < 1 || shuffle.size() > 4) {
      CompileError(&left->def, "Cannot shuffle to less than one or more than 4 output values\n");
      type = Type::Poison;
    } else
      type.sizeX = shuffle.size();
  }
  void Print(int indent) {
    left->Print(indent + 1);
    printf(".%s", shuffle.c_str());
  }
  Rvalue *left;
  std::string shuffle;
};

class ReturnExpression : public Rvalue {
public:
  ReturnExpression(Rvalue *rv)
    : Rvalue(NReturn, Type::Void, rv->def)
    , rv(rv) {
  }
  void Print(int indent) {
    printf("return ");
    rv->Print(indent + 1);
  }
  Rvalue *rv;
};

class Discard : public Rvalue {
public:
  Discard(DefLocation loc)
    : Rvalue(NDiscard, Type::Void, loc) {
  }
  void Print(int indent) {
    printf("discard");
  }
};

class BooleanChoiceExpression : public Rvalue {
public:
  BooleanChoiceExpression(Rvalue *choice, Rvalue *truth, Rvalue *lie)
    : Rvalue(NBoolChoice, truth->type, choice->def)
    , choice(choice)
    , truth(truth)
    , lie(lie) {
    type.constness = true;

    if (choice->type == Type::Poison ||
        truth->type == Type::Poison ||
        lie->type == Type::Poison)
      type = Type::Poison;
    else {
      if (choice->type.type != Type::Tbool ||
          choice->type.sizeX != 1) {
        CompileError(&choice->def, "Must use a boolean condition for choice expression\n");
        type = Type::Poison;
      } else if (truth->type != lie->type) {
        CompileError(&truth->def, "Cannot have a boolean condition with two differing types\n");
        type = Type::Poison;
      }
    }
  }
  void Print(int indent) {
    choice->Print(indent + 1);
    printf(" ? ");
    truth->Print(indent + 1);
    printf(" : ");
    lie->Print(indent + 1);
  }
  Rvalue *choice, *truth, *lie;
};

class Scope {
public:
  Scope(Scope *parent = NULL)
    : parent(parent) {
  }
  Scope *parent;
  std::map<std::string, Variable *> variables;
  Variable *getVariable(const std::string &name) {
    std::map<std::string, Variable *>::iterator it = variables.find(name);

    if (it != variables.end())
      return it->second;

    if (parent)
      return parent->getVariable(name);

    return NULL;
  }
};

class ScopeStmt : public Rvalue {
public:
  ScopeStmt(DefLocation loc, Scope *scope, std::vector<Rvalue *> statements)
    : Rvalue(NScopeStmt, Type::Void, loc)
    , scope(scope)
    , statements(statements) {
  }
  void Print(int indent) {
    printf("{\n");

    for (Rvalue *stmt : statements) {
      stmt->Print(indent + 1);
      printf(";\n");
    }

    printf("}\n");
  }
  Scope *scope;
  std::vector<Rvalue *> statements;
};

class IfStatement : public Rvalue {
public:
  IfStatement(DefLocation loc, Rvalue *condition, Rvalue *thenB, Rvalue *elseB)
    : Rvalue(NIfStmt, Type::Void, loc)
    , condition(condition)
    , thenB(thenB)
    , elseB(elseB) {
  }
  void Print(int indent) {
    printf("if (");
    condition->Print(indent);
    printf(") ");
    thenB->Print(indent);

    if (elseB) {
      printf(" else ");
      elseB->Print(indent);
    }
  }
  Rvalue *thenB, *elseB;
  Rvalue *condition;
};

class ForLoop : public Rvalue {
public:
  ForLoop(DefLocation loc, Scope *scope, Rvalue *inits, Rvalue *condition, Rvalue *update, Rvalue *loopBody)
    : Rvalue(NForLoop, Type::Void, loc)
    , scope(scope)
    , inits(inits)
    , condition(condition)
    , update(update)
    , loopBody(loopBody) {
  }
  void Print(int indent) {
    printf("for (");
    inits->Print(indent);
    printf(";");
    condition->Print(indent);
    printf(";");
    update->Print(indent);
    printf(")");
    loopBody->Print(indent);
  }
  Scope *scope;
  Rvalue *inits;
  Rvalue *condition;
  Rvalue *update;
  Rvalue *loopBody;
};

class WhileLoop : public Rvalue {
public:
  WhileLoop(DefLocation loc, Rvalue *condition, Rvalue *loopBody)
    : Rvalue(NWhileLoop, Type::Void, loc)
    , condition(condition)
    , loopBody(loopBody) {
  }
  void Print(int indent) {
    printf("while (");
    condition->Print(indent);
    printf(")");
    loopBody->Print(indent);
  }
  Rvalue *condition;
  Rvalue *loopBody;
};

class BasicBlock {
public:
  BasicBlock()
    : nextBlock(NULL)
    , refs(0) {
  }
  std::vector<Rvalue *> statements;
  BasicBlock *nextBlock;
  int refs;
};

class ConditionalJump : public Rvalue {
public:
  ConditionalJump(Rvalue *choice, BasicBlock *target)
    : Rvalue(NConditionalJump, Type::Void, choice->def)
    , choice(choice)
    , target(target) {
    if (choice->type != Type::Poison &&
        (choice->type.type != Type::Tbool ||
         choice->type.sizeX != 1))
      CompileError(&choice->def, "Can't make a conditional out of a non-boolean statement\n");
  }
  void Print(int indent) {
    printf("when ");
    choice->Print(indent + 1);
    printf(" then jump to %p\n", target);
  }
  Rvalue *choice;
  BasicBlock *target;
};

class Function {
public:
  Function(DefLocation def, const std::string &type, const std::string &name, const std::vector<FuncArg *> &arguments, Scope *scope, Rvalue *body)
    : type(Type::getType(def, type))
    , name(name)
    , arguments(arguments)
    , scope(scope)
    , body(body)
    , def(def)
    , calls(0) {
  }
  ~Function() {
    delete scope;
  }
  Type type;
  std::string name;
  std::vector<FuncArg *> arguments;
  Scope *scope;
  Rvalue *body;
  std::vector<BasicBlock *> bbs;
  std::vector<AsmBlock *> asmblocks;
  DefLocation def;
  int calls;
};

class FunctionCall : public Rvalue {
public:
  FunctionCall(DefLocation loc, Function *func, const std::vector<Rvalue *> &arguments)
    : Rvalue(NFunctionCall, func->type, loc)
    , func(func)
    , arguments(arguments) {
    func->calls++;
  }
  FunctionCall(DefLocation loc, const std::vector<Rvalue *> &arguments)
    : Rvalue(NFunctionCall, Type::Poison, loc)
    , func(NULL)
    , arguments(arguments) {
  }
  ~FunctionCall() {
    if (func)
      func->calls--;
  }
  void Print(int indent) {
    printf("%s(", func ? func->name.c_str() : "invalid");

    for (Rvalue *arg : arguments) {
      arg->Print(indent + 1);

      if (arg != arguments.back()) printf(", ");
    }

    printf(")");
  }
  static FunctionCall *CreateFromOverload(DefLocation loc, const std::string &name, std::vector<Function *> &options, const std::vector<Rvalue *> &arguments) {
    Function *f = FunctionCall::SelectOverload(loc, name, options, arguments);

    if (f)
      return new FunctionCall(loc, f, arguments);
    else
      return new FunctionCall(loc, arguments);
  }
  static Function *SelectOverload(DefLocation loc, const std::string &name, std::vector<Function *> &options, const std::vector<Rvalue *> &arguments);
  Function *func;
  std::vector<Rvalue *> arguments;
};

class Shader {
public:
  Shader()
    : in(NULL)
    , out(&in)
    , uniform(&out)
    , globalBB(DefLocation {"Builtin", 0, 0}, new Scope(&uniform), std::vector<Rvalue *>())
  , version(110) {
    CreatePredefinedStuff();
  }
  void CreatePredefinedStuff();
  Scope in, out, uniform;
  std::map<std::string, std::vector<Function *>> functions;
  ScopeStmt globalBB;
  std::map<std::string, std::string> extensions;
  std::vector<ConstVal *> cvars;
  BasicBlock *discard;
  int version;
  DefLocation loc;
};

#endif


