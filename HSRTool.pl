#!/usr/bin/perl

use strict;
use warnings FATAL => 'all';

#--------------------------- Global Vars ---------------------------
my $argHelp        = 0;
my $argBuild       = 0;
my $argOutput      = "HSRBin";
my $argEncode      = 0;
my $argDecode      = 0;
my $argBinDir      = "HSRBin";
my $argVerbose     = 0;

sub Usage {
    print "\n"
        . "Usage: \n"
        . "    -h                               Print this message\n"
        . "    -build                           Build HSR tools. This operation is assumed to be performed first after download\n"
        . "        -n <ENCODED_NODE_NUM>        Specify number of file chunks output from encoding process. Only work when -build option is specified\n"
        . "        -k <FILE_CHUNK_NUM>          Specify number of sub-files in the input file used for encoding. Only work when -build option is specified\n"
        . "        -output <DIRECTORY>          Specify directory to place binary executables. Default to be \"HSRBin\"\n"
        . "        -force_rmrf                  If output directory exists, enable this option to force remove it. Default to be disabled.\n"
        . "    -encode/decode <FILE_NAME>       Encode/Decode a file in the disk. Please ensure that HSRTool has been built\n"
        . "        -bin_dir <DIRECTORY>         Specify directory to place binary executables. Default to be \"HSRBin\"\n"
        . "        -output <DIRECTORY>      Specify directory to place encoded/decoded files\n"
        . "    -verbose                         Log informative messages\n"
        . "\n"
        . "Example: \n"
        . "    #1  Build HSRTool for parameter n=11, k=3\n"
        . "        > HSRTool.pl -build -n 11 -k 3 -output ./HSRBin/\n\n"
        . "    #2  Encode file \"dummy.txt\" after built\n"
        . "        > HSRTool.pl -encode ./dummy.txt -bin_dir ./HSRBin/ -output ./encoded_chunks/\n"
        . "        When the execution finished, dummy.txt.chunk[1-n] will be generated in the ./encoded_chunks/ dir\n\n"
        . "    #3  Decode file \"dummy.txt\" after built\n"
        . "        > HSRTool.pl -decode ./encoded_chunks/dummy.txt -bin_dir ./HSRBin/ -output ./\n\n"
        . "\n"
}

# Main
{
   Usage();
}