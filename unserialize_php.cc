#include <mysql.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <php_embed.h>
#include "pfc.h"

extern "C"{
	my_bool unserialize_php_init(UDF_INIT *initid, UDF_ARGS *args, char *message);
	char *unserialize_php(UDF_INIT *initid, UDF_ARGS *args, char *result, unsigned long *length, char *is_null, char *error);
}

my_bool unserialize_php_init(UDF_INIT *initid, UDF_ARGS *args, char *message)
{
	if (args->arg_count > 2)
	{
		strcpy(message, "too many argument: unserialize_php(php object string, php code). unserialized date is stored to $obj" );
		return 1;
	}

	if(!(args->arg_count==2 &&
			args->arg_type[0]==STRING_RESULT &&
			args->arg_type[1]==STRING_RESULT))
	{
		strcpy(message, "unserialize_php(php object string, php code). unserialized date is stored to $obj" );
		return 1;
	}

	initid->maybe_null = 1;
	initid->const_item = 0;
	initid->max_length = 16777216;

	std::string *value=new std::string();
	initid->ptr = (char *)value;
	
	return 0;
}

void unserialize_php_deinit(UDF_INIT *initid __attribute__((unused)))
{
	std::string *value = (std::string*)initid->ptr;
	delete value;
}

#define UNSERIALIZE_PHP_TARGET_STRING "unserializable_string"
#define UNSERIALIZE_PHP_TARGET_OUTPUT_VARIABLE "output"
#define UNSERIALIZE_PHP_TARGET_OBJECT "obj"
char *unserialize_php(UDF_INIT *initid, UDF_ARGS *args, char *result, unsigned long *length, char *is_null, char *error)
{
	const char* argv[] = {""};
	std::string *value = (std::string*)initid->ptr;

	PHP_EMBED_START_BLOCK(0, (char**)argv);
	pfc_set_string(
			UNSERIALIZE_PHP_TARGET_STRING,
			args->args[0],
			sizeof(UNSERIALIZE_PHP_TARGET_STRING),
			args->lengths[0]);

	if(!pfc_evalf(__func__, "$%s=unserialize($%s); $%s=%s;",
				UNSERIALIZE_PHP_TARGET_OBJECT,
				UNSERIALIZE_PHP_TARGET_STRING,
				UNSERIALIZE_PHP_TARGET_OUTPUT_VARIABLE,
				args->args[1]
				))
	{
		*is_null = 1;
		*error = 1;
		return 0;
	}
	
	if (!pfc_get_string(UNSERIALIZE_PHP_TARGET_OUTPUT_VARIABLE,
				value,
				sizeof(UNSERIALIZE_PHP_TARGET_OUTPUT_VARIABLE))) {
		*is_null = 1;
		*error = 1;
		return 0;
	}
	
	*length = strlen(value->c_str());
	if(*length==0)
	{
		*is_null = 1;
		return 0;
	}
	PHP_EMBED_END_BLOCK();

	return (char*)value->c_str();
}
