#!/usr/bin/perl
#
#	InstallEpics.pl
#
#	InstallEpics is used within makefiles to copy new versions of
#	files into a destination directory.
#	Based on installEpics shell script.
#
#	2-4-97 -kuk-
#
##########################################################

use Getopt::Std;
use File::Path;
use File::Copy;

$tool=$0;
$tool=~ s'.*[/\\].+'';	# basename
$mode=0755;

# get command line options
getopt "m";
$mode = oct ($opt_m) if ($opt_m);

#	Complain about obsolete options:
Usage("unknown option given") if ($opt_g or $opt_o or $opt_c or $opt_s);

$num_files = $#ARGV;
# at least two args required
Usage ("Nothing to install") if ($num_files < 1);

#	split args in file1 ... fileN target_dir:
@files=@ARGV[0..$num_files-1];
$install_dir=$ARGV[$num_files];
$install_dir =~ s[\\][/]g;	# maybe fix DOS-style path
$install_dir =~ s[/$][];	# remove trailing '/'
$install_dir =~ s[//][/]g;    # replace '//' by '/'

#	Do we have to create the directory?
unless (-d $install_dir)
{
	#	Create dir only if -d option given
	Usage ("$install_dir does not exist") unless ($opt_d);

	#	Create all the subdirs that lead to $install_dir
	mkpath ($install_dir, 1, 0777);
}

foreach $source ( @files )
{
	Usage ("Can't find file '$source'") unless -f $source;

	$basename=$source;
	$basename=~s'.*[/\\]'';
	$target  = "$install_dir/$basename";

	#	The Win32 filesystem seems to be 'slow',
	#	i.e. $target may look like 'up to date'
	#	unless you wait an hour.
	#	-> skip this test on WIN32 ?
	#if (-f $target  and  $^O ne "MSWin32")
	if (-f $target)
	{
		if (-M $target  <  -M $source    and
		    -C $target  <  -C $source)
		{
			print "$target is up to date\n";
			next;
		}
		else
		{
			#	remove old target, make sure it is deletable:
			chmod 0777, $target;
			unlink $target;
		}
	}

	# print "Installing $source into $install_dir\n";
	copy ($source, $target) or die "Copy failed";

	#	chmod 0555 <read-only> DOES work on WIN32, but:
	#	Another chmod 0777 to make it write- and deletable
	#	will then fail.
	#	-> you have to use Win32::SetFileAttributes
	#	   to get rid of those files from within Perl.
	#	Because the chmod is not really needed on WIN32,
	#	just skip it!
	chmod $mode, $target unless ($^O eq "MSWin32");
}

sub Usage
{
	my ($txt) = @_;

	print "Usage:\n";
	print "\t$tool [ -m mode ] file ... directory\n";
	print "\n";
	print "\t-d             Create non-exising directories\n";
	print "\t-m mode        Set the mode for the installed file";
		print " (0755 by default)\n";
	print "\tfile           Name of file\n";
	print "\tdirectory      Destination directory\n";

	print "$txt\n" if $txt;

	exit 2;
}

#	EOF installEpics.pl
