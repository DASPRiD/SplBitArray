<?php
$array = new SplBitArray(33);

for ($i = 0; $i < 33; $i++) {
    assert($array->get($i) === false, sprintf('Bit %d is not false', $i));
    $array->set($i);
    assert($array->get($i) === true, sprintf('Bit %d is not true', $i));
}

