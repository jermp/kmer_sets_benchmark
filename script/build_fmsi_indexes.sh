#!/bin/bash

results_filename_prefix=$1

source ~/miniconda3/bin/activate

# k=31

kmercamel compute -S -k 31 -M /mnt/hd2/pibiri/DNA/masked-superstrings/cod.k31.msfa -o /dev/null /mnt/hd2/pibiri/DNA/eulertigs/cod.k31.eulertigs.fa.gz
/usr/bin/time -v -a -o $results_filename_prefix.k31.build.time.log fmsi index -k 31 /mnt/hd2/pibiri/DNA/masked-superstrings/cod.k31.msfa

kmercamel compute -S -k 31 -M /mnt/hd2/pibiri/DNA/masked-superstrings/kestrel.k31.msfa -o /dev/null /mnt/hd2/pibiri/DNA/eulertigs/kestrel.k31.eulertigs.fa.gz
/usr/bin/time -v -a -o $results_filename_prefix.k31.build.time.log fmsi index -k 31 /mnt/hd2/pibiri/DNA/masked-superstrings/kestrel.k31.msfa

kmercamel compute -S -k 31 -M /mnt/hd2/pibiri/DNA/masked-superstrings/human.k31.msfa -o /dev/null /mnt/hd2/pibiri/DNA/eulertigs/human.k31.eulertigs.fa.gz
/usr/bin/time -v -a -o $results_filename_prefix.k31.build.time.log fmsi index -k 31 /mnt/hd2/pibiri/DNA/masked-superstrings/human.k31.msfa

kmercamel compute -S -k 31 -M /mnt/hd2/pibiri/DNA/masked-superstrings/ncbi-virus.k31.msfa -o /dev/null /mnt/hd2/pibiri/DNA/eulertigs/ncbi-virus.k31.eulertigs.fa.gz
/usr/bin/time -v -a -o $results_filename_prefix.k31.build.time.log fmsi index -k 31 /mnt/hd2/pibiri/DNA/masked-superstrings/ncbi-virus.k31.msfa

kmercamel compute -S -k 31 -M /mnt/hd2/pibiri/DNA/masked-superstrings/se.k31.msfa -o /dev/null /mnt/hd2/pibiri/DNA/eulertigs/se.k31.eulertigs.fa.gz
/usr/bin/time -v -a -o $results_filename_prefix.k31.build.time.log fmsi index -k 31 /mnt/hd2/pibiri/DNA/masked-superstrings/se.k31.msfa

kmercamel compute -S -k 31 -M /mnt/hd2/pibiri/DNA/masked-superstrings/hprc.k31.msfa -o /dev/null /mnt/hd2/pibiri/DNA/eulertigs/hprc.k31.eulertigs.fa.gz
/usr/bin/time -v -a -o $results_filename_prefix.k31.build.time.log fmsi index -k 31 /mnt/hd2/pibiri/DNA/masked-superstrings/hprc.k31.msfa

# k=63

kmercamel compute -S -k 63 -M /mnt/hd2/pibiri/DNA/masked-superstrings/cod.k63.msfa -o /dev/null /mnt/hd2/pibiri/DNA/eulertigs/cod.k63.eulertigs.fa.gz
/usr/bin/time -v -a -o $results_filename_prefix.k63.build.time.log fmsi index -k 63 /mnt/hd2/pibiri/DNA/masked-superstrings/cod.k63.msfa

kmercamel compute -S -k 63 -M /mnt/hd2/pibiri/DNA/masked-superstrings/kestrel.k63.msfa -o /dev/null /mnt/hd2/pibiri/DNA/eulertigs/kestrel.k63.eulertigs.fa.gz
/usr/bin/time -v -a -o $results_filename_prefix.k63.build.time.log fmsi index -k 63 /mnt/hd2/pibiri/DNA/masked-superstrings/kestrel.k63.msfa

kmercamel compute -S -k 63 -M /mnt/hd2/pibiri/DNA/masked-superstrings/human.k63.msfa -o /dev/null /mnt/hd2/pibiri/DNA/eulertigs/human.k63.eulertigs.fa.gz
/usr/bin/time -v -a -o $results_filename_prefix.k63.build.time.log fmsi index -k 63 /mnt/hd2/pibiri/DNA/masked-superstrings/human.k63.msfa

kmercamel compute -S -k 63 -M /mnt/hd2/pibiri/DNA/masked-superstrings/ncbi-virus.k63.msfa -o /dev/null /mnt/hd2/pibiri/DNA/eulertigs/ncbi-virus.k63.eulertigs.fa.gz
/usr/bin/time -v -a -o $results_filename_prefix.k63.build.time.log fmsi index -k 63 /mnt/hd2/pibiri/DNA/masked-superstrings/ncbi-virus.k63.msfa

kmercamel compute -S -k 63 -M /mnt/hd2/pibiri/DNA/masked-superstrings/se.k63.msfa -o /dev/null /mnt/hd2/pibiri/DNA/eulertigs/se.k63.eulertigs.fa.gz
/usr/bin/time -v -a -o $results_filename_prefix.k63.build.time.log fmsi index -k 63 /mnt/hd2/pibiri/DNA/masked-superstrings/se.k63.msfa

kmercamel compute -S -k 63 -M /mnt/hd2/pibiri/DNA/masked-superstrings/hprc.k63.msfa -o /dev/null /mnt/hd2/pibiri/DNA/eulertigs/hprc.k63.eulertigs.fa.gz
/usr/bin/time -v -a -o $results_filename_prefix.k63.build.time.log fmsi index -k 63 /mnt/hd2/pibiri/DNA/masked-superstrings/hprc.k63.msfa
