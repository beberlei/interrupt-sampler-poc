/* sampler extension for PHP */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <time.h>
#include <signal.h>
#include <pthread.h>
#include "php.h"
#include "ext/standard/info.h"
#include "php_sampler.h"

#define SAMPLER_G(e) php_sampler_globals.e

typedef struct _php_sampler_globals_t {
    HashTable frames;
    timer_t timer_id;
} php_sampler_globals_t;

ZEND_TLS php_sampler_globals_t php_sampler_globals;

ZEND_API void (*original_interrupt_function)(zend_execute_data *execute_data);

void sampler_interrupt_function(zend_execute_data *execute_data)
{
    zval stack;
    if (original_interrupt_function != NULL) {
        original_interrupt_function(execute_data);
    }

    zend_fetch_debug_backtrace(&stack, 0, DEBUG_BACKTRACE_IGNORE_ARGS, 0 TSRMLS_CC);

    Z_ADDREF_P(&stack);

    zend_hash_next_index_insert(&SAMPLER_G(frames), &stack);
}

static void sampler_handle(union sigval sv)
{
    EG(vm_interrupt) = 1; // changing global state across threads ok?
}

PHP_FUNCTION(sampler_start)
{
    zend_hash_init(&SAMPLER_G(frames), 0, NULL, _zval_ptr_dtor, 0);

    struct sigevent ev;
    struct itimerspec its;
    struct timespec ts;

    memset(&ev, 0, sizeof(ev));
    ev.sigev_notify = SIGEV_THREAD;
    ev.sigev_notify_function = sampler_handle;

    if (timer_create(CLOCK_MONOTONIC, &ev, &SAMPLER_G(timer_id)) != 0) {
        php_error_docref(NULL, E_WARNING, "Error in timer_create(): %s", strerror(errno));
        return FAILURE;
    }

    ts.tv_sec = 0;
    ts.tv_nsec = 1000000;

    its.it_interval = ts;
    its.it_value = ts;

    if (timer_settime(SAMPLER_G(timer_id), 0, &its, NULL) != 0) {
        php_error_docref(NULL, E_WARNING, "Error in timer_settime(): %s", strerror(errno));
    }
}

PHP_FUNCTION(sampler_stop)
{
    if (timer_delete(SAMPLER_G(timer_id)) != 0) {
        php_error_docref(NULL, E_WARNING, "Error in timer_delete(): %s", strerror(errno));
    }

    RETURN_ARR(&SAMPLER_G(frames));
}

PHP_MINIT_FUNCTION(sampler)
{
    original_interrupt_function = zend_interrupt_function;
    zend_interrupt_function = sampler_interrupt_function;

    return SUCCESS;
}

PHP_MSHUTDOWN_FUNCTION(sampler)
{
    return SUCCESS;
}

PHP_RINIT_FUNCTION(sampler)
{
#if defined(ZTS) && defined(COMPILE_DL_SAMPLER)
    ZEND_TSRMLS_CACHE_UPDATE();
#endif

    return SUCCESS;
}

PHP_RSHUTDOWN_FUNCTION(sampler)
{
    zend_hash_destroy(&SAMPLER_G(frames));

    return SUCCESS;
}

PHP_MINFO_FUNCTION(sampler)
{
    php_info_print_table_start();
    php_info_print_table_header(2, "sampler support", "enabled");
    php_info_print_table_end();
}

zend_function_entry php_sampler_functions[] = {
    PHP_FE(sampler_start, NULL)
    PHP_FE(sampler_stop, NULL)
    PHP_FE_END
};

zend_module_entry sampler_module_entry = {
    STANDARD_MODULE_HEADER,
    "sampler",
    php_sampler_functions,
    PHP_MINIT(sampler),
    PHP_MSHUTDOWN(sampler),
    PHP_RINIT(sampler),
    PHP_RSHUTDOWN(sampler),
    PHP_MINFO(sampler),
    PHP_SAMPLER_VERSION,
    STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_SAMPLER
# ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
# endif
ZEND_GET_MODULE(sampler)
#endif
