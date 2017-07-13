#include "Transformation.h"
#include "Glsl.h"

extern std::string yyfilename;

void PatchUpMain(Shader *shader) {
  // The idea behind this is:
  // Main in glsl is a "special" function; it takes no input or output
  // but it is the entry point of the shader, which as a whole takes N in variables and M out variables
  // so one way of modeling that is to transform the in/out variables
  // into arguments to main.
  if (shader->functions["main"].size() == 0) {
    CompileError(new DefLocation {yyfilename, 0, 0}, "No function called main defined!");
  } else if (shader->functions["main"].size() > 1) {
    CompileError(new DefLocation {yyfilename, 0, 0}, "More than one function called main!");
  } else {
    shader->functions["main"].front()->calls++;
    Function *main = shader->functions["main"].front();

    // TODO: honor location requests before ordering the variables
    for (std::pair<std::string, Variable *> in : shader->in.variables) {
      Variable *v = in.second;
      main->arguments.push_back(new FuncArg(v->def, "in", v->type, v->name, NULL));
    }

    for (std::pair<std::string, Variable *> out : shader->out.variables) {
      Variable *v = out.second;
      main->arguments.push_back(new FuncArg(v->def, "out", v->type, v->name, NULL));
    }

    main->scope->variables.insert(shader->in.variables.begin(), shader->in.variables.end());
    main->scope->variables.insert(shader->out.variables.begin(), shader->out.variables.end());
  }
}


