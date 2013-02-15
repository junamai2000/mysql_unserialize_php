#ifndef HAVE_PFC_H
#define HAVE_PFC_H

#include <php.h>
#include <SAPI.h>
#include <php_main.h>
#include <php_variables.h>
#include <php_ini.h>
#include <zend_ini.h>
#include <signal.h>
#include <string>

bool pfc_initialize();
void pfc_finalize();

void pfc_set_string(const char *name, const char *value, size_t name_len = 0, size_t value_len = 0);
bool pfc_get_string(const char *name, std::string *value, size_t name_len = 0);

bool pfc_eval(const char *source, const char *code);
bool pfc_evalf(const char *source, const char *fmt, ...);
#endif
