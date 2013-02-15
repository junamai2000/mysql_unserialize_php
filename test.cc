#include <stdio.h>
#include "pfc.h"

int main(int argc, char *argv[])
{
    if (!pfc_initialize()) {
        return -1;
    }

    if (!pfc_evalf(__func__, "echo \"My name is %s\n\";", argv[0])) {
        return -1;
    }

    //pfc_set_string("name", argv[0], sizeof("name"));
    // O:8:"stdClass":2:{s:1:"a";s:4:"aaaa";s:1:"b";s:4:"bbbb";}
    pfc_set_string("unserializable_string", argv[1], sizeof("unserializable_string"));

    if (!pfc_eval(__func__, "$obj=unserialize($unserializable_string);")) {
        pfc_finalize();
        return -1;
    }
    
	if (!pfc_evalf(__func__, "$output=%s;","$obj->b")) {
        return -1;
    }

    std::string *value=new  std::string;
    if (!pfc_get_string("output", value, sizeof("output"))) {
        return -1;
    }

    printf("%s\n", value->c_str());

	delete value;

    pfc_finalize();
    return 0;
}
