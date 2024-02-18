#!/usr/bin/env php
<?php
require('vendor/autoload.php');

use WebSocket\Client;

//$client = new WebSocket\Client("ws://localhost:5000");
$client = new WebSocket\Client("ws://192.168.1.38:5000");

$message = "Hello world!";
echo "> $message".PHP_EOL;
$client->text($message);
echo "< ".$client->receive().PHP_EOL;

$client->close();
?>