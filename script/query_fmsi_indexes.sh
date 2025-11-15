#!/bin/bash

results_filename_prefix=$1

./sshash_benchmark -i /mnt/hd2/pibiri/DNA/sshash-indexes/cod.k31.canon.sshash
./fmsi_benchmark -i /mnt/hd2/pibiri/DNA/masked-superstrings/cod.k31.msfa 2>> $results_filename_prefix.k31.bench.json
./fmsi_benchmark -i /mnt/hd2/pibiri/DNA/masked-superstrings/cod.k31.msfa -q /mnt/hd2/pibiri/DNA/queries/SRR12858649.fastq.gz 2>> $results_filename_prefix.k31.streaming-query-high-hit.json

./sshash_benchmark -i /mnt/hd2/pibiri/DNA/sshash-indexes/kestrel.k31.canon.sshash
./fmsi_benchmark -i /mnt/hd2/pibiri/DNA/masked-superstrings/kestrel.k31.msfa 2>> $results_filename_prefix.k31.bench.json
./fmsi_benchmark -i /mnt/hd2/pibiri/DNA/masked-superstrings/kestrel.k31.msfa -q /mnt/hd2/pibiri/DNA/queries/SRR11449743_1.fastq.gz 2>> $results_filename_prefix.k31.streaming-query-high-hit.json

./sshash_benchmark -i /mnt/hd2/pibiri/DNA/sshash-indexes/human.k31.canon.sshash
./fmsi_benchmark -i /mnt/hd2/pibiri/DNA/masked-superstrings/human.k31.msfa 2>> $results_filename_prefix.k31.bench.json
./fmsi_benchmark -i /mnt/hd2/pibiri/DNA/masked-superstrings/human.k31.msfa -q /mnt/hd2/pibiri/DNA/queries/SRR5833294.fastq.gz 2>> $results_filename_prefix.k31.streaming-query-high-hit.json

./sshash_benchmark -i /mnt/hd2/pibiri/DNA/sshash-indexes/ncbi-virus.k31.canon.sshash
./fmsi_benchmark -i /mnt/hd2/pibiri/DNA/masked-superstrings/ncbi-virus.k31.msfa 2>> $results_filename_prefix.k31.bench.json
./fmsi_benchmark -i /mnt/hd2/pibiri/DNA/masked-superstrings/ncbi-virus.k31.msfa -q /mnt/hd2/pibiri/DNA/queries/ncbi-queries.fastq.gz 2>> $results_filename_prefix.k31.streaming-query-high-hit.json

./sshash_benchmark -i /mnt/hd2/pibiri/DNA/sshash-indexes/se.k31.canon.sshash
./fmsi_benchmark -i /mnt/hd2/pibiri/DNA/masked-superstrings/se.k31.msfa 2>> $results_filename_prefix.k31.bench.json
./fmsi_benchmark -i /mnt/hd2/pibiri/DNA/masked-superstrings/se.k31.msfa -q /mnt/hd2/pibiri/DNA/queries/SRR27871075_1.fastq.gz 2>> $results_filename_prefix.k31.streaming-query-high-hit.json

./sshash_benchmark -i /mnt/hd2/pibiri/DNA/sshash-indexes/hprc.k31.canon.sshash
./fmsi_benchmark -i /mnt/hd2/pibiri/DNA/masked-superstrings/hprc.k31.msfa 2>> $results_filename_prefix.k31.bench.json
./fmsi_benchmark -i /mnt/hd2/pibiri/DNA/masked-superstrings/hprc.k31.msfa -q /mnt/hd2/pibiri/DNA/queries/SRR5833294.fastq.gz 2>> $results_filename_prefix.k31.streaming-query-high-hit.json
