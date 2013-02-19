/*
 * Astra Timer Module
 * http://cesbo.com/astra
 *
 * Copyright (C) 2012-2013, Andrey Dyldin <and@cesbo.com>
 * Licensed under the MIT license.
 */

/*
 * Module Name:
 *      timer
 *
 * Module Options:
 *      interval    - number, sets the interval between triggers, in seconds
 *      callback(instance)
 *                  - function, handler is called when the timer is triggered
 *
 * Module Methods:
 *      close(instance)
 *                  - stop timer
 */

#include <astra.h>

struct module_data_s
{
    int idx_cb;
    int idx_self;

    timer_t *timer;
};

/* callbacks */

static void timer_callback(void *arg)
{
    module_data_t *mod = arg;
    lua_rawgeti(lua, LUA_REGISTRYINDEX, mod->idx_cb);
    lua_rawgeti(lua, LUA_REGISTRYINDEX, mod->idx_self);
    lua_call(lua, 1, 0);
}

/* methods */

static int method_close(module_data_t *mod)
{
    if(mod->timer)
    {
        timer_detach(mod->timer);
        mod->timer = NULL;
    }

    luaL_unref(lua, LUA_REGISTRYINDEX, mod->idx_self);
    mod->idx_self = 0;
    luaL_unref(lua, LUA_REGISTRYINDEX, mod->idx_cb);
    mod->idx_cb = 0;

    return 0;
}

/* required */

static void module_init(module_data_t *mod)
{
    int interval = 0;
    if(!module_option_number("interval", &interval) || interval <= 0)
    {
        log_error("[timer] option 'interval' is required and must be reater than 0");
        astra_abort();
    }

    lua_getfield(lua, 2, "callback");
    if(lua_type(lua, -1) != LUA_TFUNCTION)
    {
        log_error("[timer] option 'callback' must be a function");
        astra_abort();
    }
    mod->idx_cb = luaL_ref(lua, LUA_REGISTRYINDEX);

    lua_pushvalue(lua, -1);
    mod->idx_self = luaL_ref(lua, LUA_REGISTRYINDEX);

    mod->timer = timer_attach(interval * 1000, timer_callback, mod);
}

static void module_destroy(module_data_t *mod)
{
    method_close(mod);
}

MODULE_METHODS()
{
    METHOD(close)
};

MODULE_LUA_REGISTER(timer)