#include "pfc.h"

static void pfc_sapi_error(int type, const char *fmt, ...) __attribute__ ((format (printf, 2, 3)));

void
pfc_set_string(const char *name, const char *value,
                    size_t name_len, size_t value_len)
{
    if (name_len == 0) {
        name_len = strlen(name) + 1;
    }
    if (value_len == 0) {
        value_len = strlen(value);
    }

    zval *var;
    MAKE_STD_ZVAL(var);
    ZVAL_STRINGL(var, (char *)value, value_len, 1);

    /* Optimized ZEND_SET_GLOBAL_VAR(name, var), removed name lookup */
    (void)zend_hash_update(&EG(symbol_table), (char *)name, name_len,
                           &var, sizeof(zval *), NULL);
}


bool
pfc_get_string(const char *name, std::string *value, size_t name_len)
{
    if (name_len == 0) {
        name_len = strlen(name) + 1;
    }

    zval **data = NULL;
    if (zend_hash_find(&EG(symbol_table), (char *)name, name_len,
                (void **)&data) == FAILURE) {
        pfc_sapi_error(0, "%s(): Name not found in $GLOBALS", __func__);
        return false;
    }

    if (data == NULL) {
        pfc_sapi_error(0, "%s(): Value is NULL", __func__);
        return false;
    }

    convert_to_string(*data);
    value->assign(Z_STRVAL(**data), Z_STRLEN(**data));
    return true;
}


bool
pfc_eval(const char *source, const char *code)
{
    bool result = false;

    zend_first_try {
        result = zend_eval_string((char *)code, NULL, (char *)source) == SUCCESS;
    } zend_catch {
    } zend_end_try();

    return result;
}


bool
pfc_evalf(const char *source, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    char *data = NULL;
    vspprintf(&data, 0, fmt, ap);
    bool result = pfc_eval(source, data);

    if (data != NULL) {
        efree(data);
    }

    return result;
}


static int
pfc_startup(sapi_module_struct *mod)
{
    return php_module_startup(mod, NULL, 0);
}


/* echo */
static int
pfc_ub_write(const char *str, uint len TSRMLS_DC)
{
    (void)fputs(str, stdout);
    return 0;
}


static void
pfc_log_message(char *str)
{
    (void)fprintf(stderr, "%s\n", str);
}


static void
pfc_sapi_error(int type, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    (void)vfprintf(stderr, fmt, ap);
    (void)fputs("\n", stderr);
    va_end(ap);
}

#define pfc_shutdown php_module_shutdown_wrapper

sapi_module_struct g_pfc_module = {
	"foobar",                       /* name */
	"FOOBAR-PHP Interface",         /* pretty name */
	pfc_startup,             /* startup */
	pfc_shutdown,            /* shutdown */
	NULL,                           /* activate */
	NULL,                           /* deactivate */
	pfc_ub_write,            /* unbuffered write */
	NULL,                           /* flush */
	NULL,                           /* get uid */
	NULL,                           /* getenv */
	php_error,                      /* error handler */
	NULL,                           /* header handler */
	NULL,                           /* send headers handler */
	NULL,                           /* send header handler */
	NULL,                           /* read POST data */
	NULL,                           /* read Cookies */
	NULL,                           /* register server variables */
	pfc_log_message,         /* Log message */
  
	STANDARD_SAPI_MODULE_PROPERTIES
};


bool 
pfc_initialize()
{
    g_pfc_module.sapi_error = pfc_sapi_error;
	zend_llist global_vars;

#ifdef ZTS /* zend threat safety */
	zend_compiler_globals *compiler_globals;
	zend_executor_globals *executor_globals;
	php_core_globals *core_globals;
	sapi_globals_struct *sapi_globals;
	void ***tsrm_ls;

    tsrm_startup(1, 1, 0, NULL);

    compiler_globals = ts_resource(compiler_globals_id);
    executor_globals = ts_resource(executor_globals_id);
    core_globals = ts_resource(core_globals_id);
    sapi_globals = ts_resource(sapi_globals_id);
    tsrm_ls = ts_resource(0);
    *ptsrm_ls = tsrm_ls;
#endif

	signal(SIGPIPE, SIG_IGN);

    sapi_startup(&g_pfc_module);

    if (g_pfc_module.startup(&g_pfc_module) == FAILURE) {
      return false;
    }

    g_pfc_module.executable_location = (char *)__func__;

    zend_llist_init(&global_vars, sizeof(char *), NULL, 0);  

    /* Set some Embedded PHP defaults */
    SG(options) |= SAPI_OPTION_NO_CHDIR;

    if (php_request_startup(TSRMLS_C) == FAILURE) {
      php_module_shutdown(TSRMLS_C);
      return false;
    }

    SG(headers_sent) = 1;
    SG(request_info).no_headers = 1;
    php_register_variable("PHP_SELF", "-", NULL TSRMLS_CC);

    return true;
}


void 
pfc_finalize()
{
  	php_request_shutdown((void *) 0);
	php_module_shutdown(TSRMLS_C);
	sapi_shutdown();
#ifdef ZTS
    tsrm_shutdown();
#endif
}

/*
 * $Log$
 */
