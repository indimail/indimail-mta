<?php
	if ($argc > 1)
		$argument = $argv[1];
	else
		$argument = "indimail.org";
	$fp = fsockopen("ssl://".$argument, 993, $errno, $errstr, 30);
	if ($fp) {
		echo "Success\n";
		fclose($fp);
	} else {
		echo "Fail: errstr=\"$errstr\", errno=\"$errno\"\n";
	}
