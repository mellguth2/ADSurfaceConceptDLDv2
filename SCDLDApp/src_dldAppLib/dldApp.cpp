#include "dldApp.h"
#include <vector>
#include <string>

struct Globals
{
  std::vector<ParamMeta> params;
} G;

struct ParamMeta
{
  scdldapp_param_info1 base;
  union
  {
    scdldapp_param_int32_info1 int32;
    scdldapp_param_float64_info1 float64;
  };
  std::vector<std::string> choices;
};



void scdldapp_initlib()
{
  if (G.params.size() == 0)
  {
    scdldapp_init_params();
  }
}

void scdldapp_init_params()
{

}
