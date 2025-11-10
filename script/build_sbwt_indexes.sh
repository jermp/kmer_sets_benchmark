#!/bin/bash

results_filename_prefix=$1

/usr/bin/time -v -a -o $results_filename_prefix-build.time.log ./sbwt_build/bin/sbwt build -i /mnt/hd2/pibiri/DNA/eulertigs/cod.k31.eulertigs.fa.gz -k 31 -m 16 -t 64 -d /mnt/hd2/pibiri/DNA/tmp_dir/ --verbose -o /mnt/hd2/pibiri/DNA/sbwt-indexes/cod.k31.sbwt
/usr/bin/time -v -a -o $results_filename_prefix-build.time.log ./sbwt_build/bin/sbwt build -i /mnt/hd2/pibiri/DNA/eulertigs/cod.k31.eulertigs.fa.gz -k 31 -m 16 -t 64 -d /mnt/hd2/pibiri/DNA/tmp_dir/ --add-reverse-complements --verbose -o /mnt/hd2/pibiri/DNA/sbwt-indexes/cod.k31.rc.sbwt

/usr/bin/time -v -a -o $results_filename_prefix-build.time.log ./sbwt_build/bin/sbwt build -i /mnt/hd2/pibiri/DNA/eulertigs/kestrel.k31.eulertigs.fa.gz -k 31 -m 16 -t 64 -d /mnt/hd2/pibiri/DNA/tmp_dir/ --verbose -o /mnt/hd2/pibiri/DNA/sbwt-indexes/kestrel.k31.sbwt
/usr/bin/time -v -a -o $results_filename_prefix-build.time.log ./sbwt_build/bin/sbwt build -i /mnt/hd2/pibiri/DNA/eulertigs/kestrel.k31.eulertigs.fa.gz -k 31 -m 16 -t 64 -d /mnt/hd2/pibiri/DNA/tmp_dir/ --add-reverse-complements --verbose -o /mnt/hd2/pibiri/DNA/sbwt-indexes/kestrel.k31.rc.sbwt

/usr/bin/time -v -a -o $results_filename_prefix-build.time.log ./sbwt_build/bin/sbwt build -i /mnt/hd2/pibiri/DNA/eulertigs/human.k31.eulertigs.fa.gz -k 31 -m 16 -t 64 -d /mnt/hd2/pibiri/DNA/tmp_dir/ --verbose -o /mnt/hd2/pibiri/DNA/sbwt-indexes/human.k31.sbwt
/usr/bin/time -v -a -o $results_filename_prefix-build.time.log ./sbwt_build/bin/sbwt build -i /mnt/hd2/pibiri/DNA/eulertigs/human.k31.eulertigs.fa.gz -k 31 -m 16 -t 64 -d /mnt/hd2/pibiri/DNA/tmp_dir/ --add-reverse-complements --verbose -o /mnt/hd2/pibiri/DNA/sbwt-indexes/human.k31.rc.sbwt

/usr/bin/time -v -a -o $results_filename_prefix-build.time.log ./sbwt_build/bin/sbwt build -i /mnt/hd2/pibiri/DNA/eulertigs/hprc.k31.eulertigs.fa.gz -k 31 -m 16 -t 64 -d /mnt/hd2/pibiri/DNA/tmp_dir/ --verbose -o /mnt/hd2/pibiri/DNA/sbwt-indexes/hprc.k31.sbwt
/usr/bin/time -v -a -o $results_filename_prefix-build.time.log ./sbwt_build/bin/sbwt build -i /mnt/hd2/pibiri/DNA/eulertigs/hprc.k31.eulertigs.fa.gz -k 31 -m 16 -t 64 -d /mnt/hd2/pibiri/DNA/tmp_dir/ --add-reverse-complements --verbose -o /mnt/hd2/pibiri/DNA/sbwt-indexes/hprc.k31.rc.sbwt

/usr/bin/time -v -a -o $results_filename_prefix-build.time.log ./sbwt_build/bin/sbwt build -i /mnt/hd2/pibiri/DNA/eulertigs/ec.k31.eulertigs.fa.gz -k 31 -m 16 -t 64 -d /mnt/hd2/pibiri/DNA/tmp_dir/ --verbose -o /mnt/hd2/pibiri/DNA/sbwt-indexes/ec.k31.sbwt
/usr/bin/time -v -a -o $results_filename_prefix-build.time.log ./sbwt_build/bin/sbwt build -i /mnt/hd2/pibiri/DNA/eulertigs/ec.k31.eulertigs.fa.gz -k 31 -m 16 -t 64 -d /mnt/hd2/pibiri/DNA/tmp_dir/ --add-reverse-complements --verbose -o /mnt/hd2/pibiri/DNA/sbwt-indexes/ec.k31.rc.sbwt

/usr/bin/time -v -a -o $results_filename_prefix-build.time.log ./sbwt_build/bin/sbwt build -i /mnt/hd2/pibiri/DNA/eulertigs/se.k31.eulertigs.fa.gz -k 31 -m 16 -t 64 -d /mnt/hd2/pibiri/DNA/tmp_dir/ --verbose -o /mnt/hd2/pibiri/DNA/sbwt-indexes/se.k31.sbwt
/usr/bin/time -v -a -o $results_filename_prefix-build.time.log ./sbwt_build/bin/sbwt build -i /mnt/hd2/pibiri/DNA/eulertigs/se.k31.eulertigs.fa.gz -k 31 -m 16 -t 64 -d /mnt/hd2/pibiri/DNA/tmp_dir/ --add-reverse-complements --verbose -o /mnt/hd2/pibiri/DNA/sbwt-indexes/se.k31.rc.sbwt
