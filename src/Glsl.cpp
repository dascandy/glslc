#include "Glsl.h"
#include <stdarg.h>
#include <string.h>
#include <math.h>

extern bool compileErrors;

const char spacebuf[33] = "                                ";
const char *spaces(int n) {
  return spacebuf + strlen(spacebuf) - n;
}

Type Type::Poison(Type::Tpoison, 1, 1, true);
Type Type::Empty(Type::Tempty, 1, 1, true);
Type Type::Void(Type::Tvoid, 1, 1, true);

std::map<std::string, Type> knownTypes = {
  std::pair<std::string, Type>("float", Type(Type::Tfloat, 1, 1, false)),
  std::pair<std::string, Type>("vec2", Type(Type::Tfloat, 2, 1, false)),
  std::pair<std::string, Type>("vec3", Type(Type::Tfloat, 3, 1, false)),
  std::pair<std::string, Type>("vec4", Type(Type::Tfloat, 4, 1, false)),
  std::pair<std::string, Type>("void", Type(Type::Tvoid, 1, 1, true)),
  std::pair<std::string, Type>("int", Type(Type::Tint, 1, 1, false)),
  std::pair<std::string, Type>("uint", Type(Type::Tuint, 1, 1, false)),
  std::pair<std::string, Type>("bool", Type(Type::Tbool, 1, 1, false)),
};

void CompileError(DefLocation *loc, const char *err, ...) {
  if (loc)
    printf("%s:%lu:%lu: ", loc->fileName.c_str(), loc->lineno, loc->offs);

  va_list args;
  va_start(args, err);
  vprintf(err, args);
  va_end(args);
  compileErrors = true;
}

std::string Type::asString() const {
  for (std::pair<const std::string, Type> &ent : knownTypes) {
    if (*this == ent.second) return ent.first;
  }

  return "Unknown type";
}

DefLocation Builtin {"builtin", 0, 0};

void Shader::CreatePredefinedStuff() {
  in.variables.insert(std::make_pair("gl_Position", new Variable(Builtin, "vec4", "gl_Position", -1)));
  in.variables.insert(std::make_pair("gl_FragCoord", new Variable(Builtin, "vec4", "gl_FragCoord", -1)));
  out.variables.insert(std::make_pair("gl_FragColor", new Variable(Builtin, "vec4", "gl_FragColor", -1)));
  out.variables.insert(std::make_pair("gl_FragData", new Variable(Builtin, "vec4", "gl_FragData", -1)));
  Rvalue *blocks = new ScopeStmt(Builtin, new Scope(), std::vector<Rvalue *>());
  functions["vec4"].push_back(new Function(Builtin, "vec4", "vec4", std::vector<FuncArg *> {new FuncArg(Builtin, "in", "float", "", NULL), new FuncArg(Builtin, "in", "float", "", NULL), new FuncArg(Builtin, "in", "float", "", NULL), new FuncArg(Builtin, "in", "float", "", NULL)} ,
  new Scope(), blocks));
  functions["vec4"].push_back(new Function(Builtin, "vec4", "vec4", std::vector<FuncArg *> {new FuncArg(Builtin, "in", "vec2", "", NULL), new FuncArg(Builtin, "in", "float", "", NULL), new FuncArg(Builtin, "in", "float", "", NULL)} ,
  new Scope(), blocks));
  functions["vec4"].push_back(new Function(Builtin, "vec4", "vec4", std::vector<FuncArg *> {new FuncArg(Builtin, "in", "float", "", NULL), new FuncArg(Builtin, "in", "vec2", "", NULL), new FuncArg(Builtin, "in", "float", "", NULL)} ,
  new Scope(), blocks));
  functions["vec4"].push_back(new Function(Builtin, "vec4", "vec4", std::vector<FuncArg *> {new FuncArg(Builtin, "in", "float", "", NULL), new FuncArg(Builtin, "in", "float", "", NULL), new FuncArg(Builtin, "in", "vec2", "", NULL)} ,
  new Scope(), blocks));
  functions["vec4"].push_back(new Function(Builtin, "vec4", "vec4", std::vector<FuncArg *> {new FuncArg(Builtin, "in", "vec2", "", NULL), new FuncArg(Builtin, "in", "vec2", "", NULL)} , new Scope(),
  blocks));
  functions["vec4"].push_back(new Function(Builtin, "vec4", "vec4", std::vector<FuncArg *> {new FuncArg(Builtin, "in", "vec3", "", NULL), new FuncArg(Builtin, "in", "float", "", NULL)} , new Scope(),
  blocks));
  functions["vec4"].push_back(new Function(Builtin, "vec4", "vec4", std::vector<FuncArg *> {new FuncArg(Builtin, "in", "float", "", NULL), new FuncArg(Builtin, "in", "vec3", "", NULL)} , new Scope(),
  blocks));
  functions["vec4"].push_back(new Function(Builtin, "vec4", "vec4", std::vector<FuncArg *> {new FuncArg(Builtin, "in", "vec4", "", NULL)} , new Scope(), blocks));
  functions["vec4"].push_back(new Function(Builtin, "vec4", "vec4", std::vector<FuncArg *> {new FuncArg(Builtin, "in", "float", "", NULL)} , new Scope(), blocks));
  functions["vec3"].push_back(new Function(Builtin, "vec3", "vec3", std::vector<FuncArg *> {new FuncArg(Builtin, "in", "float", "", NULL), new FuncArg(Builtin, "in", "float", "", NULL), new FuncArg(Builtin, "in", "float", "", NULL)} ,
  new Scope(), blocks));
  functions["vec3"].push_back(new Function(Builtin, "vec3", "vec3", std::vector<FuncArg *> {new FuncArg(Builtin, "in", "vec2", "", NULL), new FuncArg(Builtin, "in", "float", "", NULL)} , new Scope(),
  blocks));
  functions["vec3"].push_back(new Function(Builtin, "vec3", "vec3", std::vector<FuncArg *> {new FuncArg(Builtin, "in", "float", "", NULL), new FuncArg(Builtin, "in", "vec2", "", NULL)} , new Scope(),
  blocks));
  functions["vec3"].push_back(new Function(Builtin, "vec3", "vec3", std::vector<FuncArg *> {new FuncArg(Builtin, "in", "vec3", "", NULL)} , new Scope(), blocks));
  functions["vec3"].push_back(new Function(Builtin, "vec3", "vec3", std::vector<FuncArg *> {new FuncArg(Builtin, "in", "float", "", NULL)} , new Scope(), blocks));
  functions["vec2"].push_back(new Function(Builtin, "vec2", "vec2", std::vector<FuncArg *> {new FuncArg(Builtin, "in", "float", "", NULL), new FuncArg(Builtin, "in", "float", "", NULL)} , new Scope(),
  blocks));
  functions["vec2"].push_back(new Function(Builtin, "vec2", "vec2", std::vector<FuncArg *> {new FuncArg(Builtin, "in", "float", "", NULL)} , new Scope(), blocks));
  functions["vec2"].push_back(new Function(Builtin, "vec2", "vec2", std::vector<FuncArg *> {new FuncArg(Builtin, "in", "vec2", "", NULL)} , new Scope(), blocks));
  functions["sin"].push_back(new Function(Builtin, "float", "sin", std::vector<FuncArg *> {new FuncArg(Builtin, "in", "float", "", NULL)} , new Scope(), blocks));
  functions["sin"].push_back(new Function(Builtin, "vec2", "sin", std::vector<FuncArg *> {new FuncArg(Builtin, "in", "vec2", "", NULL)} , new Scope(), blocks));
  functions["sin"].push_back(new Function(Builtin, "vec3", "sin", std::vector<FuncArg *> {new FuncArg(Builtin, "in", "vec3", "", NULL)} , new Scope(), blocks));
  functions["sin"].push_back(new Function(Builtin, "vec4", "sin", std::vector<FuncArg *> {new FuncArg(Builtin, "in", "vec4", "", NULL)} , new Scope(), blocks));
  functions["cos"].push_back(new Function(Builtin, "float", "cos", std::vector<FuncArg *> {new FuncArg(Builtin, "in", "float", "", NULL)} , new Scope(), blocks));
  functions["cos"].push_back(new Function(Builtin, "vec2", "cos", std::vector<FuncArg *> {new FuncArg(Builtin, "in", "vec2", "", NULL)} , new Scope(), blocks));
  functions["cos"].push_back(new Function(Builtin, "vec3", "cos", std::vector<FuncArg *> {new FuncArg(Builtin, "in", "vec3", "", NULL)} , new Scope(), blocks));
  functions["cos"].push_back(new Function(Builtin, "vec4", "cos", std::vector<FuncArg *> {new FuncArg(Builtin, "in", "vec4", "", NULL)} , new Scope(), blocks));
  functions["abs"].push_back(new Function(Builtin, "float", "abs", std::vector<FuncArg *> {new FuncArg(Builtin, "in", "float", "", NULL)} , new Scope(), blocks));
  functions["abs"].push_back(new Function(Builtin, "vec2", "abs", std::vector<FuncArg *> {new FuncArg(Builtin, "in", "vec2", "", NULL)} , new Scope(), blocks));
  functions["abs"].push_back(new Function(Builtin, "vec3", "abs", std::vector<FuncArg *> {new FuncArg(Builtin, "in", "vec3", "", NULL)} , new Scope(), blocks));
  functions["abs"].push_back(new Function(Builtin, "vec4", "abs", std::vector<FuncArg *> {new FuncArg(Builtin, "in", "vec4", "", NULL)} , new Scope(), blocks));
  functions["mod"].push_back(new Function(Builtin, "float", "mod", std::vector<FuncArg *> {new FuncArg(Builtin, "in", "float", "", NULL), new FuncArg(Builtin, "in", "float", "", NULL)} , new Scope(),
  blocks));
  functions["mod"].push_back(new Function(Builtin, "vec2", "mod", std::vector<FuncArg *> {new FuncArg(Builtin, "in", "vec2", "", NULL), new FuncArg(Builtin, "in", "vec2", "", NULL)} , new Scope(),
  blocks));
  functions["mod"].push_back(new Function(Builtin, "vec3", "mod", std::vector<FuncArg *> {new FuncArg(Builtin, "in", "vec3", "", NULL), new FuncArg(Builtin, "in", "vec3", "", NULL)} , new Scope(),
  blocks));
  functions["mod"].push_back(new Function(Builtin, "vec4", "mod", std::vector<FuncArg *> {new FuncArg(Builtin, "in", "vec4", "", NULL), new FuncArg(Builtin, "in", "vec4", "", NULL)} , new Scope(),
  blocks));
  functions["pow"].push_back(new Function(Builtin, "float", "pow", std::vector<FuncArg *> {new FuncArg(Builtin, "in", "float", "", NULL), new FuncArg(Builtin, "in", "float", "", NULL)} , new Scope(),
  blocks));
  functions["pow"].push_back(new Function(Builtin, "vec2", "pow", std::vector<FuncArg *> {new FuncArg(Builtin, "in", "vec2", "", NULL), new FuncArg(Builtin, "in", "vec2", "", NULL)} , new Scope(),
  blocks));
  functions["pow"].push_back(new Function(Builtin, "vec3", "pow", std::vector<FuncArg *> {new FuncArg(Builtin, "in", "vec3", "", NULL), new FuncArg(Builtin, "in", "vec3", "", NULL)} , new Scope(),
  blocks));
  functions["pow"].push_back(new Function(Builtin, "vec4", "pow", std::vector<FuncArg *> {new FuncArg(Builtin, "in", "vec4", "", NULL), new FuncArg(Builtin, "in", "vec4", "", NULL)} , new Scope(),
  blocks));
  functions["min"].push_back(new Function(Builtin, "float", "min", std::vector<FuncArg *> {new FuncArg(Builtin, "in", "float", "", NULL), new FuncArg(Builtin, "in", "float", "", NULL)} , new Scope(),
  blocks));
  functions["min"].push_back(new Function(Builtin, "vec2", "min", std::vector<FuncArg *> {new FuncArg(Builtin, "in", "vec2", "", NULL), new FuncArg(Builtin, "in", "vec2", "", NULL)} , new Scope(),
  blocks));
  functions["min"].push_back(new Function(Builtin, "vec2", "min", std::vector<FuncArg *> {new FuncArg(Builtin, "in", "vec2", "", NULL), new FuncArg(Builtin, "in", "float", "", NULL)} , new Scope(),
  blocks));
  functions["min"].push_back(new Function(Builtin, "vec2", "min", std::vector<FuncArg *> {new FuncArg(Builtin, "in", "float", "", NULL), new FuncArg(Builtin, "in", "vec2", "", NULL)} , new Scope(),
  blocks));
  functions["min"].push_back(new Function(Builtin, "vec3", "min", std::vector<FuncArg *> {new FuncArg(Builtin, "in", "vec3", "", NULL), new FuncArg(Builtin, "in", "vec3", "", NULL)} , new Scope(),
  blocks));
  functions["min"].push_back(new Function(Builtin, "vec3", "min", std::vector<FuncArg *> {new FuncArg(Builtin, "in", "vec3", "", NULL), new FuncArg(Builtin, "in", "float", "", NULL)} , new Scope(),
  blocks));
  functions["min"].push_back(new Function(Builtin, "vec3", "min", std::vector<FuncArg *> {new FuncArg(Builtin, "in", "float", "", NULL), new FuncArg(Builtin, "in", "vec3", "", NULL)} , new Scope(),
  blocks));
  functions["min"].push_back(new Function(Builtin, "vec4", "min", std::vector<FuncArg *> {new FuncArg(Builtin, "in", "vec4", "", NULL), new FuncArg(Builtin, "in", "vec4", "", NULL)} , new Scope(),
  blocks));
  functions["min"].push_back(new Function(Builtin, "vec4", "min", std::vector<FuncArg *> {new FuncArg(Builtin, "in", "vec4", "", NULL), new FuncArg(Builtin, "in", "float", "", NULL)} , new Scope(),
  blocks));
  functions["min"].push_back(new Function(Builtin, "vec4", "min", std::vector<FuncArg *> {new FuncArg(Builtin, "in", "float", "", NULL), new FuncArg(Builtin, "in", "vec4", "", NULL)} , new Scope(),
  blocks));
  functions["max"].push_back(new Function(Builtin, "float", "max", std::vector<FuncArg *> {new FuncArg(Builtin, "in", "float", "", NULL), new FuncArg(Builtin, "in", "float", "", NULL)} , new Scope(),
  blocks));
  functions["max"].push_back(new Function(Builtin, "vec2", "max", std::vector<FuncArg *> {new FuncArg(Builtin, "in", "vec2", "", NULL), new FuncArg(Builtin, "in", "vec2", "", NULL)} , new Scope(),
  blocks));
  functions["max"].push_back(new Function(Builtin, "vec2", "max", std::vector<FuncArg *> {new FuncArg(Builtin, "in", "vec2", "", NULL), new FuncArg(Builtin, "in", "float", "", NULL)} , new Scope(),
  blocks));
  functions["max"].push_back(new Function(Builtin, "vec2", "max", std::vector<FuncArg *> {new FuncArg(Builtin, "in", "float", "", NULL), new FuncArg(Builtin, "in", "vec2", "", NULL)} , new Scope(),
  blocks));
  functions["max"].push_back(new Function(Builtin, "vec3", "max", std::vector<FuncArg *> {new FuncArg(Builtin, "in", "vec3", "", NULL), new FuncArg(Builtin, "in", "vec3", "", NULL)} , new Scope(),
  blocks));
  functions["max"].push_back(new Function(Builtin, "vec3", "max", std::vector<FuncArg *> {new FuncArg(Builtin, "in", "vec3", "", NULL), new FuncArg(Builtin, "in", "float", "", NULL)} , new Scope(),
  blocks));
  functions["max"].push_back(new Function(Builtin, "vec3", "max", std::vector<FuncArg *> {new FuncArg(Builtin, "in", "float", "", NULL), new FuncArg(Builtin, "in", "vec3", "", NULL)} , new Scope(),
  blocks));
  functions["max"].push_back(new Function(Builtin, "vec4", "max", std::vector<FuncArg *> {new FuncArg(Builtin, "in", "vec4", "", NULL), new FuncArg(Builtin, "in", "vec4", "", NULL)} , new Scope(),
  blocks));
  functions["max"].push_back(new Function(Builtin, "vec4", "max", std::vector<FuncArg *> {new FuncArg(Builtin, "in", "vec4", "", NULL), new FuncArg(Builtin, "in", "float", "", NULL)} , new Scope(),
  blocks));
  functions["max"].push_back(new Function(Builtin, "vec4", "max", std::vector<FuncArg *> {new FuncArg(Builtin, "in", "float", "", NULL), new FuncArg(Builtin, "in", "vec4", "", NULL)} , new Scope(),
  blocks));
  functions["distance"].push_back(new Function(Builtin, "float", "distance", std::vector<FuncArg *> {new FuncArg(Builtin, "in", "vec2", "", NULL), new FuncArg(Builtin, "in", "vec2", "", NULL)} , new
  Scope(), blocks));
  functions["atan"].push_back(new Function(Builtin, "float", "atan", std::vector<FuncArg *> {new FuncArg(Builtin, "in", "float", "", NULL), new FuncArg(Builtin, "in", "float", "", NULL)} , new Scope(),
  blocks));
  functions["cross"].push_back(new Function(Builtin, "vec3", "cross", std::vector<FuncArg *> {new FuncArg(Builtin, "in", "vec3", "", NULL), new FuncArg(Builtin, "in", "vec3", "", NULL)} , new Scope(),
  blocks));
  functions["dot"].push_back(new Function(Builtin, "float", "dot", std::vector<FuncArg *> {new FuncArg(Builtin, "in", "vec2", "", NULL), new FuncArg(Builtin, "in", "vec2", "", NULL)} , new Scope(),
  blocks));
  functions["dot"].push_back(new Function(Builtin, "float", "dot", std::vector<FuncArg *> {new FuncArg(Builtin, "in", "vec3", "", NULL), new FuncArg(Builtin, "in", "vec3", "", NULL)} , new Scope(),
  blocks));
  functions["dot"].push_back(new Function(Builtin, "float", "dot", std::vector<FuncArg *> {new FuncArg(Builtin, "in", "vec4", "", NULL), new FuncArg(Builtin, "in", "vec4", "", NULL)} , new Scope(),
  blocks));
  functions["exp"].push_back(new Function(Builtin, "float", "exp", std::vector<FuncArg *> {new FuncArg(Builtin, "in", "float", "", NULL)} , new Scope(), blocks));
  functions["floor"].push_back(new Function(Builtin, "float", "floor", std::vector<FuncArg *> {new FuncArg(Builtin, "in", "float", "", NULL)} , new Scope(), blocks));
  functions["floor"].push_back(new Function(Builtin, "vec2", "floor", std::vector<FuncArg *> {new FuncArg(Builtin, "in", "vec2", "", NULL)} , new Scope(), blocks));
  functions["floor"].push_back(new Function(Builtin, "vec3", "floor", std::vector<FuncArg *> {new FuncArg(Builtin, "in", "vec3", "", NULL)} , new Scope(), blocks));
  functions["floor"].push_back(new Function(Builtin, "vec4", "floor", std::vector<FuncArg *> {new FuncArg(Builtin, "in", "vec4", "", NULL)} , new Scope(), blocks));
  functions["fract"].push_back(new Function(Builtin, "float", "fract", std::vector<FuncArg *> {new FuncArg(Builtin, "in", "float", "", NULL)} , new Scope(), blocks));
  functions["fract"].push_back(new Function(Builtin, "vec2", "fract", std::vector<FuncArg *> {new FuncArg(Builtin, "in", "vec2", "", NULL)} , new Scope(), blocks));
  functions["fract"].push_back(new Function(Builtin, "vec3", "fract", std::vector<FuncArg *> {new FuncArg(Builtin, "in", "vec3", "", NULL)} , new Scope(), blocks));
  functions["fract"].push_back(new Function(Builtin, "vec4", "fract", std::vector<FuncArg *> {new FuncArg(Builtin, "in", "vec4", "", NULL)} , new Scope(), blocks));
  functions["length"].push_back(new Function(Builtin, "float", "length", std::vector<FuncArg *> {new FuncArg(Builtin, "in", "vec2", "", NULL)} , new Scope(), blocks));
  functions["length"].push_back(new Function(Builtin, "float", "length", std::vector<FuncArg *> {new FuncArg(Builtin, "in", "vec3", "", NULL)} , new Scope(), blocks));
  functions["length"].push_back(new Function(Builtin, "float", "length", std::vector<FuncArg *> {new FuncArg(Builtin, "in", "vec4", "", NULL)} , new Scope(), blocks));
  functions["normalize"].push_back(new Function(Builtin, "vec2", "normalize", std::vector<FuncArg *> {new FuncArg(Builtin, "in", "vec2", "", NULL)} , new Scope(), blocks));
  functions["normalize"].push_back(new Function(Builtin, "vec3", "normalize", std::vector<FuncArg *> {new FuncArg(Builtin, "in", "vec3", "", NULL)} , new Scope(), blocks));
  functions["mix"].push_back(new Function(Builtin, "float", "mix", std::vector<FuncArg *> {new FuncArg(Builtin, "in", "float", "", NULL), new FuncArg(Builtin, "in", "float", "", NULL), new FuncArg(Builtin, "in", "float", "", NULL)} ,
  new Scope(), blocks));
  functions["smoothstep"].push_back(new Function(Builtin, "float", "smoothstep", std::vector<FuncArg *> {new FuncArg(Builtin, "in", "float", "", NULL), new FuncArg(Builtin, "in", "float", "", NULL), new FuncArg(Builtin, "in", "float", "", NULL)} ,
  new Scope(), blocks));
  functions["step"].push_back(new Function(Builtin, "float", "step", std::vector<FuncArg *> {new FuncArg(Builtin, "in", "float", "", NULL), new FuncArg(Builtin, "in", "float", "", NULL)} , new Scope(),
  blocks));
  functions["clamp"].push_back(new Function(Builtin, "float", "clamp", std::vector<FuncArg *> {new FuncArg(Builtin, "in", "float", "", NULL), new FuncArg(Builtin, "in", "float", "", NULL), new FuncArg(Builtin, "in", "float", "", NULL)} ,
  new Scope(), blocks));
  functions["float"].push_back(new Function(Builtin, "float", "float", std::vector<FuncArg *> {new FuncArg(Builtin, "in", "uint", "", NULL)}, new Scope(), blocks));
  functions["float"].push_back(new Function(Builtin, "float", "float", std::vector<FuncArg *> {new FuncArg(Builtin, "in", "int", "", NULL)}, new Scope(), blocks));
  functions["float"].push_back(new Function(Builtin, "float", "float", std::vector<FuncArg *> {new FuncArg(Builtin, "in", "float", "", NULL)}, new Scope(), blocks));
}

static int valueFor(char c) {
  if (c >= '0' && c <= '9') return c - '0';

  if (c >= 'a' && c <= 'z') return c - 'a' + 10;

  if (c >= 'A' && c <= 'Z') return c - 'A' + 10;

  return -1;
}

static unsigned int parseDigitSequence(const char *&valstr) {
  int radix;
  unsigned int value = 0;
  int val = 0;

  if (valstr[0] == '0' && (valstr[1] == 'x' || valstr[1] == 'X')) {
    radix = 16;
    valstr += 2;
  } else if (valstr[0] == '0') {
    radix = 8;
    valstr++;
  } else
    radix = 10;

  while ((val = valueFor(*valstr)) != -1) {
    value = value * radix + val;
    valstr++;
  }

  return value;
}

void ImmediateIntExpression::SetValue(const char *value) {
  val = parseDigitSequence(value);
}

void ImmediateUintExpression::SetValue(const char *value) {
  val = parseDigitSequence(value);
}

void ImmediateFloatExpression::SetValue(const char *value) {
  unsigned long long mantissa = 0;
  int exponent = 0;
  bool afterDot = false;
  bool inNumber = true;

  while (inNumber) {
    if (*value >= '0' && *value <= '9') {
      mantissa = mantissa * 10 + (*value) - '0';

      if (afterDot) exponent--;
    } else if (*value == '.')
      afterDot = true;
    else
      break;

    value++;
  }

  if (*value == 'e' || *value == 'E') {
    bool exponentNeg = false;
    value++;

    if (*value == '-') {
      exponentNeg = true;
      value++;
    } else if (*value == '+')
      value++;

    while (*value >= '0' && *value <= '9') {
      exponent = exponent * 10 + (*value) - '0';
      value++;
    }
  }

  if (*value == 'l' || *value == 'L')
    precise = true;

  val = (long double)mantissa * powl(10, exponent);
}

Type Type::getType(DefLocation loc, std::string typeName) {
  bool constVal = false;

  if (typeName.substr(0, 5) == "const") {
    typeName = typeName.substr(6);
    constVal = true;
  }

  std::map<std::string, Type>::iterator it = knownTypes.find(typeName);

  if (it != knownTypes.end()) {
    Type t = it->second;
    t.constness = constVal;
    return t;
  } else {
    CompileError(&loc, "Undefined type: |%s|\n", typeName.c_str());
    return Type::Poison;
  }
}

Type Expression::GetType(Rvalue *left, operation op, Rvalue *right) {
  // Weird operator; just return the right type regardless of what it is.
  // Comma is the only well-defined operator in case of Poison, so that check is done first.
  if (op == comma) {
    if (right)
      return right->type;
  }

  if (left->type == Type::Poison ||
      right && right->type == Type::Poison)
    return Type::Poison;

  if (op == plus) {
    if (left->type.type == right->type.type) {
      if (left->type.type == Type::Tfloat ||
          left->type.type == Type::Tint ||
          left->type.type == Type::Tuint ||
          left->type.type == Type::Tdouble)
        return left->type;
    }

    CompileError(&left->def, "Can't add %s and %s together\n", left->type.asString().c_str(), right->type.asString().c_str());
    return Type::Poison;
  }

  if (op == minus) {
    if (right == NULL)
      return left->type;

    if (left->type == right->type) {
      if (left->type.type == Type::Tfloat ||
          left->type.type == Type::Tint ||
          left->type.type == Type::Tuint ||
          left->type.type == Type::Tdouble)
        return left->type;
    }

    CompileError(&left->def, "Can't subtract %s from %s\n", right->type.asString().c_str(), left->type.asString().c_str());
    return Type::Poison;
  }

  if (op == times) {
    if (left->type == right->type) {
      if (left->type.type == Type::Tfloat ||
          left->type.type == Type::Tint ||
          left->type.type == Type::Tuint ||
          left->type.type == Type::Tdouble)
        return left->type;
    }

    if (left->type.type == right->type.type &&
        left->type.sizeY == 1 &&
        right->type.sizeY == 1 &&
        (left->type.sizeX == 1 ||
         right->type.sizeX == 1)) {
      if (left->type.type == Type::Tfloat ||
          left->type.type == Type::Tint ||
          left->type.type == Type::Tuint ||
          left->type.type == Type::Tdouble)
        return Type(left->type.type, std::max(left->type.sizeX, right->type.sizeX), 1, true);
    }

    CompileError(&left->def, "Can't multiply %s and %s together\n", left->type.asString().c_str(), right->type.asString().c_str());
    return Type::Poison;
  }

  if (op == divide) {
    if (left->type == right->type) {
      if (left->type.type == Type::Tfloat ||
          left->type.type == Type::Tint ||
          left->type.type == Type::Tuint ||
          left->type.type == Type::Tdouble)
        return left->type;
    }

    if (left->type.type == right->type.type &&
        left->type.sizeY == 1 &&
        right->type.sizeY == 1 &&
        right->type.sizeX == 1) {
      if (left->type.type == Type::Tfloat ||
          left->type.type == Type::Tint ||
          left->type.type == Type::Tuint ||
          left->type.type == Type::Tdouble)
        return left->type;
    }

    CompileError(&left->def, "Can't divide %s by %s\n", left->type.asString().c_str(), right->type.asString().c_str());
    return Type::Poison;
  }

  if (op == assign) {
    if (left->type.type == right->type.type) {
      if (left->type.type == Type::Tfloat ||
          left->type.type == Type::Tint ||
          left->type.type == Type::Tuint ||
          left->type.type == Type::Tdouble)
        return left->type;
    }

    CompileError(&left->def, "Can't assign %s from %s\n", left->type.asString().c_str(), right->type.asString().c_str());
    return Type::Poison;
  }

  if (op == comp_gt ||
      op == comp_lt ||
      op == comp_ge ||
      op == comp_le) {
    if (left->type.sizeX == 1 &&
        left->type.sizeY == 1 &&
        right->type.sizeX == 1 &&
        right->type.sizeY == 1 &&
        left->type.type == right->type.type)
      return Type(Type::Tbool, 1, 1, true);

    CompileError(&left->def, "Can't compare %s to %s\n", left->type.asString().c_str(), right->type.asString().c_str());
    return Type::Poison;
  }

  if (op == comp_eq ||
      op == comp_ne) {
    if (left->type == right->type)
      return Type(Type::Tbool, 1, 1, true);

    CompileError(&left->def, "Can't compare %s to %s\n", left->type.asString().c_str(), right->type.asString().c_str());
    return Type::Poison;
  }

  if (op == invert) {
    if (left->type.type == Type::Tbool)
      return left->type;

    CompileError(&left->def, "Can't invert an expression of type %s\n", left->type.asString().c_str());
    return Type::Poison;
  }

  if (op == neg) {
    if (left->type.type == Type::Tfloat ||
        left->type.type == Type::Tint ||
        left->type.type == Type::Tuint ||
        left->type.type == Type::Tdouble)
      return left->type;

    CompileError(&left->def, "Can't negate a %s expression\n", left->type.asString().c_str());
    return Type::Poison;
  }

  if (op == booland) {
    if (left->type.type == Type::Tbool &&
        right->type.type == Type::Tbool &&
        left->type.sizeX == right->type.sizeX &&
        left->type.sizeY == right->type.sizeY)
      return left->type;

    CompileError(&left->def, "Cannot use binary AND on %s and %s\n", left->type.asString().c_str(), right->type.asString().c_str());
    return Type::Poison;
  }

  if (op == postincr) {
    if (left->type == Type(Type::Tint, 1, 1, false) ||
        left->type == Type(Type::Tuint, 1, 1, false)) {
      Type t = left->type;
      t.constness = true;
      return t;
    }

    CompileError(&left->def, "Cannot use post-fix ++ on %s\n", left->type.asString().c_str());
    return Type::Poison;
  }

  CompileError(&left->def, "Unsupported operation: %d\n", op);
  return Type::Poison;
}

static void PrintExpressionTypeList(const std::vector<Rvalue *> &args) {
  CompileError(NULL, "(");

  for (Rvalue *r : args) {
    if (r)
      CompileError(NULL, r->type.asString().c_str());

    if (r != args.back())
      CompileError(NULL, ", ");
  }

  CompileError(NULL, ")");
}

static void PrintFunctionPrototype(Function *func) {
  CompileError(&func->def, func->name.c_str());
  CompileError(NULL, "(");

  for (FuncArg *fa : func->arguments) {
    if (fa)
      CompileError(NULL, fa->type.asString().c_str());

    if (fa != func->arguments.back())
      CompileError(NULL, ", ");
  }

  CompileError(NULL, ")\n");
}

Function *FunctionCall::SelectOverload(DefLocation loc, const std::string &name, std::vector<Function *> &options, const std::vector<Rvalue *> &arguments) {
  // If one of the arguments is poisoned, poison the return.
  for (Rvalue *arg : arguments) {
    if (arg && arg->type == Type::Poison)
      return NULL;
  }

  std::vector<Function *> choices;

  for (Function *f : options) {
    if (f->arguments.size() != arguments.size())
      continue;

    bool isAcceptable = true;

    for (size_t i = 0; i < arguments.size(); ++i) {
      if (arguments[i] == NULL && f->arguments[i] == NULL) {
      } else if (arguments[i]->type.canConvertTo(f->arguments[i]->type)) {
      } else
        isAcceptable = false;
    }

    if (!isAcceptable) continue;

    choices.push_back(f);
  }

  if (choices.size() == 0) {
    CompileError(&loc, "Can't find function %s", name.c_str());
    PrintExpressionTypeList(arguments);
    CompileError(NULL, "\n");

    if (options.size() > 0) {
      CompileError(&loc, "Could have considered:\n");

      for (Function *f : options)
        PrintFunctionPrototype(f);
    }
  } else if (choices.size() == 1)
    return choices.front();
  else {
    CompileError(&loc, "Ambiguous function call on %s", name.c_str());
    PrintExpressionTypeList(arguments);
    CompileError(NULL, "\n");
    CompileError(&loc, "Options are:\n");

    for (Function *f : choices)
      PrintFunctionPrototype(f);
  }

  return NULL;
}


