/* Copyright 2022 Surface Concept GmbH */

#include "dldApp.h"
#include <string>
#include <memory>
#include <utility>
#include <unordered_map>
#include "glue.hpp"
#include "DLD.hpp"
#include "dldApp_inline_config.hpp"

struct User
{
  std::shared_ptr<Glue<DLD> > glue_; // shared_ptr can delete via base class ptr
  template <typename ... Args>
  User(Args&& ... args)
    : glue_(std::make_shared<DLD>(std::forward<Args>(args)...))
  {
  }
};

std::unordered_map<int, std::unique_ptr<User>> users_;

namespace {
  // call a function of the User class using an object identified by user_id
  template <typename F, typename... Args>
  int user_call(const int user_id, F&& f, Args&&... args)
  {
    try {
      auto& userptr = users_.at(user_id);
      return (*(userptr->glue_).*f)(std::forward<Args>(args)...);
    } catch (const std::out_of_range&) {
      return SCDLDAPP_ERR_NO_SUCH_USER;
    }
  }
} // namespace

int scdldapp_write_int32(int user_id, size_t pidx, int value)
{
  return user_call(user_id, &Glue<DLD>::write_int, pidx, value);
}

int scdldapp_read_int32(int user_id, size_t pidx, int *value)
{
  return user_call(user_id, &Glue<DLD>::read_int, pidx, value);
}

int scdldapp_write_float64(int user_id, size_t pidx, double value)
{
  return user_call(user_id, &Glue<DLD>::write_float64, pidx, value);
}

int scdldapp_read_float64(int user_id, size_t pidx, double *value)
{
  return user_call(user_id, &Glue<DLD>::read_float64, pidx, value);
}

int scdldapp_write_string(int user_id, size_t pidx, const char *value)
{
  return user_call(user_id, &Glue<DLD>::write_string, pidx, value);
}

int scdldapp_read_string(int user_id, size_t pidx, size_t *len, char *value)
{
  return user_call(user_id, &Glue<DLD>::read_string, pidx, len, value);
}

int scdldapp_write_enum(int user_id, size_t pidx, int value)
{
  return user_call(user_id, &Glue<DLD>::write_enum, pidx, value);
}

int scdldapp_read_enum(int user_id, size_t pidx, int *value)
{
  return user_call(user_id, &Glue<DLD>::read_enum, pidx, value);
}

int scdldapp_set_callback_int32(int user_id, void* priv, scdldapp_cb_int32 cb)
{
  return user_call(user_id, &Glue<DLD>::set_callback_int32, priv, cb);
}

int scdldapp_set_callback_float64(int user_id, void* priv, scdldapp_cb_float64 cb)
{
  return user_call(user_id, &Glue<DLD>::set_callback_float64, priv, cb);
}

int scdldapp_set_callback_string(int user_id, void* priv, scdldapp_cb_string cb)
{
  return user_call(user_id, &Glue<DLD>::set_callback_string, priv, cb);
}

int scdldapp_set_callback_enum(int user_id, void* priv, scdldapp_cb_enum cb)
{
  return user_call(user_id, &Glue<DLD>::set_callback_enum, priv, cb);
}

int scdldapp_create_user()
{
  static const int MAX_USERS = 100;
  for (int i = 0; i < MAX_USERS; i++) {
    if (users_.find(i) == users_.end()) {
      users_.emplace(i, std::make_unique<User>());
      return i;
    }
  }
  return SCDLDAPP_ERR_MAX_USERS;
}

void scdldapp_delete_user(int user_id)
{
  users_.erase(user_id);
}

const char *scdldapp_get_param_config_json()
{
  return param_config_data; // let's trust lib users not to write into our char
                            // buffer
}

void scdldapp_get_version(int *ver_maj, int *ver_min, int *ver_pat)
{
  if (ver_maj) *ver_maj = SC_DLD_APP_LIB_VER_MAJ;
  if (ver_min) *ver_min = SC_DLD_APP_LIB_VER_MIN;
  if (ver_pat) *ver_pat = SC_DLD_APP_LIB_VER_PAT;
}
