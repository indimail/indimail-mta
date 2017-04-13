#!/bin/sh
setsebool -P httpd_can_network_connect 1
setsebool -P httpd_read_user_content 1
