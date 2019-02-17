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
    baz();
}
function baz() {
    usleep(100);
}

for ($i = 0; $i < 1000; $i++) {
    foo();
    baz();
}

$data = sampler_stop();

$samples = [];
foreach ($data as $frames) {
    $name = "";
    foreach ($frames as $frame) {
        if ($name) {
            $name .= "==>";
        }

        if (isset($frame['function'])) {
            if (isset($frame['class'])) {
                $name .= $frame['class'];
            }
            $name .= $frame['function'];
        } else if (isset($frame['file'])) {
            $name .= basename(dirname($frame['file'])) . '/' . basename($frame['file']);
        } else {
            $name .= "{internal}";
        }
    }

    if (!isset($samples[$name])) {
        $samples[$name] = 0;
    }
    $samples[$name]++;
}
var_dump($samples);
--EXPECTF--
