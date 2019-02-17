--TEST--
sampler: test
--FILE--
<?php

sampler_start(0.01);

function foo() {
    bar();
}
function bar() {
    usleep(100);
}

for ($i = 0; $i < 1000; $i++) {
    foo();
}

$data = sampler_stop();
var_dump($data);
--EXPECTF--
