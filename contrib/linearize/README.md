# Linearize
Construct a linear, no-fork, best version of the blockchain.

## Step 0: Install x16r_hash

https://github.com/brian112358/x16r_hash

## Step 1: Download hash list

    $ ./linearize-hashes.py linearize.cfg > hashlist.txt

Required configuration file settings for linearize-hashes:
* RPC: rpcuser, rpcpassword

Optional config file setting for linearize-hashes:
* RPC: host, port
* Block chain: min_height, max_height

## Step 2: Copy local block data

    $ ./linearize-data.py linearize.cfg

Required configuration file settings:
* "input": bitcoind blocks/ directory containing blkNNNNN.dat
* "hashlist": text file containing list of block hashes, linearized-hashes.py
output.
* "output_file" for bootstrap.dat or "output" for output directory for linearized blocks/blkNNNNN.dat output

Optional config file setting for linearize-data:
* "netmagic": network magic number (default is '3f7c8b6d', mainnet)
* "genesis": genesis block hash (default is '00000b720c7944a2446d30dabd344fa4596787f728da752b2a36f6c01ec867fe', mainnet)
* "max_out_sz": maximum output file size (default 100 \* 1000 \* 1000)
* "split_timestamp": Split files when a new month is first seen, in addition to
reaching a maximum file size.
* "file_timestamp": Set each file's last-modified time to that of the
most recent block in that file.
