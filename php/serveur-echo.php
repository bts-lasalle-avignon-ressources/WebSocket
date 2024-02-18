#!/usr/bin/env php
<?php
require('vendor/autoload.php');

use WebSocket\Server;

$options = array_merge([
    'port'          => 5000,
    'timeout'       => 200,
    'filter'        => ['text', 'binary', 'ping', 'pong', 'close'],
], getopt('', ['port:', 'timeout:', 'debug']));
$serveur = new WebSocket\Server($options);
$serveur->accept();

$message = $serveur->receive();
echo "< $message".PHP_EOL;
$serveur->text($message);
echo "> $message".PHP_EOL;

$serveur->close();
?>

