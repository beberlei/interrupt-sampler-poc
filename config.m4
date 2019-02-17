dnl config.m4 for extension sampler

PHP_ARG_ENABLE(sampler, whether to enable sampler support,
[  --enable-sampler          Enable sampler support], no)

if test "$PHP_sampler" != "no"; then
    dnl Timers require real-time and pthread library on Linux and not
    dnl supported on other platforms
    AC_SEARCH_LIBS([timer_create], [rt], [
      PHP_EVAL_LIBLINE($LIBS, SAMPLER_SHARED_LIBADD)
    ])
    AC_SEARCH_LIBS([sem_init], [pthread], [
      PHP_EVAL_LIBLINE($LIBS, SAMPLER_SHARED_LIBADD)
    ])

  PHP_SUBST(SAMPLER_SHARED_LIBADD)
  PHP_NEW_EXTENSION(sampler, sampler.c, $ext_shared,, -DZEND_ENABLE_STATIC_TSRMLS_CACHE=1)
fi
