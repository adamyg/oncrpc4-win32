#!/usr/bin/perl -w
# -*- mode: perl; -*-
# $Id: mkdir_p.pl,v 1.1 2022/05/15 07:04:11 cvsuser Exp $
#
# "mkdir -p" emulation.
#
use strict;
use warnings 'all';

sub mkdir_p {
    my $dir = shift;

    $dir =~ s|/*\Z(?!\n)||s;
    return if (-d $dir);

    if ($dir =~ m|[^/]/|s) {
	my $parent = $dir;
	$parent =~ s|[^/]*\Z(?!\n)||s;
	mkdir_p($parent);
    }

    unless (mkdir($dir, 0777)) {
	return if (-d $dir);
	die "Cannot create directory $dir: $!\n";
    }

    print "created directory `$dir'\n";
}

my $arg;

foreach $arg (@ARGV) {
    $arg =~ tr|\\|/|;
    mkdir_p($arg);
}

#end

