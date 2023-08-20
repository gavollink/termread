#!/usr/bin/perl
##############################################################################
# generate_u9_matches.pl
#
# Utility to help write terminal name match lists for termread.c
# not very useful on it's own.
#
# For license, see embaed at the bottom of termread.c
#
# What this does is find all the terminal types on a system and
# ask 'tput' for the 'u9' (user9) output which is where the string
# telling how to ask the terminal for its capabilities exists.
# It ignores terms without 'u9' and sorts all terminals by each 'u9'
# string and puts those strings into c functions that are compatible
# with termread.c
#
##
use strict;
use warnings qw{ all };

use English qw( -no_match_vars );

MAIN: {
    my $ret = {};
    my %u9cmd;

    $ret = get_terms();

    foreach my $term ( sort keys %{$ret} ) {
        my $out = read_tcap( $ret->{$term}, $term );
        if ($out) {
            my @chr     = split( //, $out );
            my $newline = q{};
            foreach my $ltr (@chr) {
                if ( 033 == ord($ltr) ) {
                    $newline .= "\\E";
                }
                elsif ( 0x20 > ord($ltr) ) {
                    $newline .= sprintf( "\\0%o", ord($ltr) );
                }
                elsif ( 0177 <= ord($ltr) ) {
                    $newline .= sprintf( "\\0%o", ord($ltr) );
                }
                else {
                    $newline .= $ltr;
                }
            } ## end foreach my $ltr (@chr)
            if ( exists $u9cmd{$newline} ) {
                push @{ $u9cmd{$newline} }, $term;
            } else {
                $u9cmd{$newline} = [ $term ];
            }
        } ## end if ($out)
    } ## end foreach my $term ( sort keys...)

    foreach my $u9 ( sort keys %u9cmd ) {
        my $longname = $u9;
        $longname =~ s/\\E/esc_/g;
        $longname =~ s/\?/question_/g;
        $longname =~ s/\[/openbracket_/g;
        $longname =~ s/\{/openbrace_/g;
        printf "\n";
        print "#" x 40, "\n";
        printf "int is_%s ( const char * )\n", $longname;
        printf "{\n";
        printf "    /*******\n";
        printf "     *   SELF ID ( user9 ) |%s| term list\n", $u9;
        printf "     */\n";
        printf "    const char *term_list[] = {\n";
        foreach my $term ( sort @{ $u9cmd{$u9} } ) {
            printf "        \"%s\",\n", $term;
        }
        printf "        \"\\000\"\n";
        printf "    };\n";
        printf "    return is_matchlist( term, term_list );\n";
        printf "}\n";
        printf "\n";
    }
} ## end MAIN:

sub read_tcap
{
    my $path = shift;
    my $term = shift;
    my $fh   = undef;

    $ENV{'TERMINFO_DIRS'} = $path;
    $ENV{'TERM'}          = $term;
    open( $fh, "tput u9|" );
    if ( ! $fh ) {
        printf *STDERR, "Unable to open tput: %s\n", $path, $OS_ERROR;
        return 0;
    }
    my $line;
    while ( $line = readline($fh) ) {
        if ( $fh->eof() ) {
            last;
        }
    }
    close($fh);
    if ( ! $line || -z "$line" ) {
        return 0;
    }
    return $line;

} ## end sub read_tcap

sub read_files
{
    my $path = shift;
    my $ret  = shift;
    my $ph   = undef;

    opendir $ph, $path;
    if ( ! $ph ) {
        printf *STDERR, "Unable to open %s: %s\n", $path, $OS_ERROR;
        exit(1);
    }
    while ( my $entry = readdir $ph ) {
        next if ( $entry =~ m/^\./ );
        my $fullentry = "$path/$entry";
        if ( -f $fullentry ) {
            $ret->{$entry} = $path;
        }
    } ## end while ( my $entry = readdir...)
    closedir $ph;

    return $ret;
} ## end sub read_files

sub read_term_subpaths
{
    my $path = shift;
    my $ret  = shift || {};
    my $ph   = undef;
    my @list = ();

    opendir $ph, $path;
    if ( ! $ph ) {
        printf *STDERR, "Unable to open %s: %s\n", $path, $OS_ERROR;
        exit(1);
    }
    while ( my $entry = readdir $ph ) {
        next if ( $entry =~ m/^\./ );
        my $fullentry = "$path/$entry";
        if ( -d $fullentry ) {
            push @list, $fullentry;
        }
    } ## end while ( my $entry = readdir...)
    closedir $ph;

    foreach my $p (@list) {
        read_files( $p, $ret );
    }

    return $ret;
} ## end sub read_term_subpaths

sub get_terms
{
    my $ret      = shift || {};
    my $termdirs = $ENV{'TERMINFO_DIRS'};

    if ( -z $termdirs ) {
        $termdirs = join(
            ':',
            (   sprintf( "%s/.terminfo", $ENV{'HOME'} ),
                "/lib/terminfo",
                "/etc/terminfo",
                "/usr/share/terminfo"
            )
        );
    } ## end if ( -z $termdirs )

    for my $dir ( reverse split( /:/, $termdirs ) ) {
        if ( -d $dir ) {
            read_term_subpaths( $dir, $ret );
        }
    }

    return $ret;
} ## end sub get_terms
