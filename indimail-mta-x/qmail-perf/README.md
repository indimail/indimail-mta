# Performance Graphs for indimail-mta

## Overview

The script qmail-perf-test uses mail.c to inject emails. The program mail.c forks multiple parallel process to inject mail. Each email has a subject having the email number. A .qmail-test is installed which triggers a fifo /tmp/qmail-test when the last email injected by mail.c is received.  qmail-perf-test waits on this fifo using readfifo. After the trigger is pulled, It then uses waits for all emails injected to arrive. When they do, the script zoverall from qmailanalog is used to generate statistics.  All values are recorded in a csv file which is then used in google sheets to generate the graphs.			

## Hardware

* Sony VAIO laptop from 2012
* CPU - Intel(R) Core(TM) i3-3120M CPU @ 2.50GHz
	* CPU MHz - 1407.227
	* CPU Cache Size - 3072 KB
	* CPU Model - 58
* Hard Disk - Toshiba 2.5" HDD MQ01ABD
	* Device Model TOSHIBA MQ01ABD050
	* Serial Number Z2MLF2S7S
	* LU WWN Device Id 5 000039 477186ae0
	* Firmware Version AX0A3H
	* User Capacity 500,107,862,016 bytes [500 GB]
	* Sector Sizes 512 bytes logical, 4096 bytes physical
	* Rotation Rate 5400 rpm
	* Form Factor 2.5 inches
	* Device is In smartctl database
	* ATA Version is ATA8-ACS
* Memory
	* 8Gb DDR3
* OS FC 35
* Filesystem
	* ext4 for binaries
	* zfs for qmail queue
* Software
	* daemontools from openSUSE [Build Service Repo](https://software.opensuse.org//download.html?project=home%3Ambhangui&package=daemontools)
	* indimail-mta (dynamic-queue branch) from source [indimail-mta](https://github.com/mbhangui/indimail-mta)
	* netqmail from source [netqmail](http://netqmail.org/)
	* netqmail-exttodo from [netqmail source](http://netqmail.org) + [exttodo patch](https://github.com/bruceg/qmail-patches/blob/master/ext_todo-20030105.patch)
	* notqmail from source [notqmail](https://github.com/notqmail/notqmail)
	* s/qmail from source [s/qmail](https://www.fehcom.de/sqmail/sqmail.html)
	* qmail-perf-test, testfs script
	* compiled binaries of sub.c, mail.c, loadavg.c, readfifo.c

## What was Tested

### MTA / Methods

MTA/Method|Description
-----------|------
indimail-mta|indimail-mta using traditional queue trigger method using lock/trigger with a dedicated todo processor.  This mta has a runtime configurable directory split and bigtodo
ipc|indimail-mta using POSIX IPC, nameley message queues and shared memory for communication between qmail-queue, qmail-todo and qmail-send. This mta has a runtime configurable directory split and bigtodo
qmta|indimail-mta having qmail-send+qmail-lspawn-qmail-rspawn+qmail-clean as a single binary. This mta doesn't have a separate todo processor. This mta has a runtime configurable directory split and bigtodo
slowq|indimail-mta having qmail-send with rate control delivery. This mta doesn't have a separate todo processor. This mta has a runtime configurable directory split and bigtodo
compat|indimail-mta using IPC but which can respond to both lock/trigger and POSIX IPC message queue. This mta has a separate todo processor. This mta has a runtime configurable directory split and bigtodo
netqmail|Probably the second fork of qmail to the best of my knowledge. This mta doesn't have a separate todo processor and doesn't use the bigtodo directory.
exttodo|This patch gives external todo processor to netqmail
notqmail|Collaborative open-source successor to qmail and netqmail. This mta doesn't have a separate todo processor and doesn't use the bigtodo directory.
s/qmail|Another fork of qmail. This has a separate todo processor and bigtodo directory
batch-ipc|This lock/trigger method in indimail-mta sends delivery instructions to qmail-lspawn/qmail-rspawn in batches of a number defined by TODO_CHUNK_SIZE environment variable. See this comment in qmail-send.c by djb /* XXX: could allow a bigger buffer; say 10 recipients */
batch-trigger|This IPC method in indimail-mta sends delivery instructions to qmail-lspawn/qmail-rspawn in batches of a number defined by TODO_CHUNK_SIZE environment variable. See this comment in qmail-send.c by djb /* XXX: could allow a bigger buffer; say 10 recipients */

### Directory Split

	* Directory split of 23 (this is what qmail, netqmail, notqmail, s/qmail use). This is a compile time configuration.
	* Directory split of 151. This is what indimail-mta uses as default, but is runtime configurable by setting CONFSPLIT environment variable

### Sync Methods

	* fsync enabled/disabled. qmail, netqail, notqmail, s/qmail have fsync system call always enabled in qmail-queue.c, qmail-send.c, qmail-local.c. In indimail-mta, fsync can be enabled/disabled by setting USE_FSYNC environment variable
	* syncdir enabled/disabled. indimail-mta uses a modified version of Bruce Guenter syncdir patch for qmail
	* fdatasync instead of fsync. This can be turned on in indimail-mta by setting USE_FDATASYNC instead of USE_FSYNC environment variable.

## Observations

	* qmail based MTAs that use an external todo processor demonstrate a lower qtime
	* external todo processor has a remarkable impact on the local concurrency. The concurrency never reaches high values with high inject rates.
	* processing todo in batches has a significant impact on qmail-send performance and delivery times by as much as 30%. But this has an impact on the delivery of the first email.
	* Increasting directory split has negligible effect in qmail-perf test and filesystem test
	* statically linked binaries give much better performance. With dynamic linking, indimail-mta performs the worst amongst all MTAs.
	* When delivery rate increase inject rate decreases
	* The biggest impact on local delivery rate are the fsync() calls. Changing fsync() to fdatasync() did not result in the delivery rate. Disabling fsync() resulted in local deliveries increasing by 6x.
		* Disabling fsync, ext4 gave the best performance in the test carried out
		* Using fsync, zfs gave the best performance in the tests carried out

## Results

Results on [Google Sheet](https://docs.google.com/spreadsheets/d/1Dfr1c1RXh18Lc47fmGymTRV5nL9DRviS9Gy8kqH5iZM/edit?usp=sharing)
