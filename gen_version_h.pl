#!/usr/bin/env perl
use strict;
use warnings;
use POSIX qw(strftime);
use IPC::Open3;
use Symbol qw(gensym);

my $output = shift @ARGV // 'version.h';

sub command_output {
    my (@cmd) = @_;
    my $err = gensym;
    my ($out, $pid);
    eval { $pid = open3(undef, $out, $err, @cmd); };
    return undef if $@;
    local $/;
    my $stdout = <$out>;
    my $stderr = <$err>; # drain it, but intentionally do not display it
    waitpid($pid, 0);
    return undef if $? != 0 || !defined $stdout;
    $stdout =~ s/[\r\n]+\z//;
    return length($stdout) ? $stdout : undef;
}

sub c_escape {
    my ($s) = @_;
    $s =~ s/\\/\\\\/g;
    $s =~ s/"/\\"/g;
    return $s;
}

my ($version, $source);

# GitHub Actions gives us the release tag directly on tag-triggered builds.
if (($ENV{GITHUB_REF_TYPE} // '') eq 'tag' &&
    defined($ENV{GITHUB_REF_NAME}) && length($ENV{GITHUB_REF_NAME})) {
    $version = $ENV{GITHUB_REF_NAME};
    $source = 'github-tag';
}

# Outside Actions, use an exact tag on HEAD if there is one.
if (!defined $version) {
    my $tag = command_output('git', 'describe', '--tags', '--exact-match', 'HEAD');
    if (defined $tag) {
        $version = $tag;
        $source = 'git-tag';
    }
}

# With no tag, identify the source by commit.  Prefix with 'g' so this can
# never be confused with a normal v-prefixed release tag.
if (!defined $version) {
    my $sha = $ENV{GITHUB_SHA};
    if (!defined($sha) || $sha !~ /^[0-9a-fA-F]{7,}$/) {
        $sha = command_output('git', 'rev-parse', '--verify', 'HEAD');
    }
    if (defined($sha) && $sha =~ /^([0-9a-fA-F]{7,})$/) {
        my $short = substr(lc($1), 0, 12);
        $version = 'g' . $short;
        $source = 'git-commit';
    }
}

# A source archive may have no Git metadata at all.  Use a UTC ISO-8601 date,
# prefixed with 'd' so it is distinguishable from both tags and commit IDs.
if (!defined $version) {
    $version = 'd' . strftime('%Y-%m-%dT%H:%M:%SZ', gmtime(time));
    $source = 'date';
}

my $text = <<"HEADER";
#ifndef UGE_VERSION_H
#define UGE_VERSION_H

#define UGE_VERSION "@{[c_escape($version)]}"
#define UGE_VERSION_SOURCE "@{[c_escape($source)]}"

#endif // UGE_VERSION_H
HEADER

my $old;
if (open my $in, '<', $output) {
    local $/;
    $old = <$in>;
    close $in;
}

# Avoid touching the file when the version did not change; this prevents a
# no-op version probe from forcing uge.cpp to rebuild on every make.
exit 0 if defined($old) && $old eq $text;

my $tmp = "$output.tmp.$$";
open my $out, '>', $tmp or die "$0: cannot write $tmp: $!\n";
print {$out} $text;
close $out or die "$0: cannot close $tmp: $!\n";
rename $tmp, $output or die "$0: cannot rename $tmp to $output: $!\n";
