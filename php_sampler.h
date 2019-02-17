/* sampler extension for PHP */

#ifndef PHP_SAMPLER_H
# define PHP_SAMPLER_H

extern zend_module_entry sampler_module_entry;
# define phpext_sampler_ptr &sampler_module_entry

# define PHP_SAMPLER_VERSION "0.0.1"

# if defined(ZTS) && defined(COMPILE_DL_SAMPLER)
ZEND_TSRMLS_CACHE_EXTERN()
# endif

#endif	/* PHP_SAMPLER_H */
