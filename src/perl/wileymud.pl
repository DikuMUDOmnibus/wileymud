#!/usr/bin/perl -w

package main;

=head1 NAME

wileymud - The venerable WileyMUD III, rewritten in perl.

=head1 SYNOPSYS

Please see L<Mud::Options> for usage details.

=head1 DESCRIPTION

WileyMUD was the first text MUD I ever played.  It was a very
popular DikuMUD that ran at Western Michigan University for
several years, and ate the lives and free time of far too many
students.

A few years after the original team shut it down, I managed to
ressurect it briefly.  During my tenure, the code was stabilized
to the point of being quite solid and reliable, and some new
features were added... some good, some not so good.

In any case, I've neglected it over the years because modern
languages have given me a severe dislike for dealing with strings
as pointers and arrays, and having to constantly fiddle with
memory management.

Thus, I finally decided to try to rewrite the entire game in
a langauge that avoids those issues... the one I know best is
perl.  So, this project is born.


=cut

use strict;
use warnings;
use English -no_match_vars;
use Data::Dumper;

use Time::HiRes qw( time sleep alarm );

use Mud::Options;
use Mud::Logger qw(:all);
use Mud::Signals;
use Mud::GameTime;
use Mud::Weather;
use Mud::Terminal;
use Mud::Theme;
use Mud::Documents;
use Mud::Comm;

my $options = Mud::Options->new(@ARGV);

Mud::Logger::setup_options($options);

warn "Testing";

my $start_time = time();
my $done = undef;

if (defined $options->libdir) {
    my $libdir = $options->libdir;

    die "Cannot change directory to $libdir" if ! -d $libdir;
    chdir $libdir;
}

if (defined $options->pidfile) {
    my $pidfile = $options->pidfile;

    log_boot "Dumping PID to %s", $pidfile;
    open FP, ">$pidfile" or die "Cannot open PID file $pidfile";
    print FP "$$\n";
    close FP;
}

switch_logfile $options->logfile if defined $options->logfile;

log_boot "Game port: %d", $options->gameport;
log_boot "Random number seed value: %d", srand;

my @descriptor_list = ();

log_boot "Boot DB -- BEGIN.";

my $time_daemon = Mud::GameTime->new();
my $weather_daemon = Mud::Weather->new($time_daemon);
my $term = Mud::Terminal->new('ansi', 132, 40);
my $theme = Mud::Theme->new('default', $term);

log_debug $theme->colorize( "%^RED%^Hello%^RESET%^\n" );
log_debug $theme->colorize( "%^DAMAGE%^Heal me!%^RESET%^  %^HEALING%^Done.%^RESET%^\n" );
log_debug $theme->terminal_type;
log_debug $theme->width;
log_debug $theme->height;

our $documents = Mud::Documents->new();

log_info "News: %s\n", Dumper($documents->news);
log_info "Credits: %s\n", Dumper($documents->credits);
log_info "MOTD: %s\n", Dumper($documents->motd);
log_info "WIZMOTD: %s\n", Dumper($documents->wizmotd);
log_info "wizlist: %s\n", Dumper($documents->wizlist);
log_info "Info: %s\n", Dumper($documents->info);
log_info "Story: %s\n", Dumper($documents->story);
log_info "License: %s\n", Dumper($documents->license);
log_info "Greeting: %s\n", Dumper($documents->greeting);

my $comm = Mud::Comm->new($options);

#    log_boot("- Reading login menu");
#    file_to_prompt(LOGIN_MENU_FILE, login_menu);
#    log_boot("- Reading sex menu");
#    file_to_prompt(SEX_MENU_FILE, sex_menu);
#    log_boot("- Reading race menu");
#    file_to_prompt(RACE_MENU_FILE, race_menu);
#    log_boot("- Reading class menu");
#    file_to_prompt(CLASS_MENU_FILE, class_menu);
#    log_boot("- Reading race help");
#    file_to_prompt(RACE_HELP_FILE, race_help);
#    log_boot("- Reading class help");
#    file_to_prompt(CLASS_HELP_FILE, class_help);
#    log_boot("- Reading story");
#    file_to_string(STORY_FILE, the_story);
#    log_boot("- Reading suicide warning");
#    file_to_prompt(SUICIDE_WARN_FILE, suicide_warn);
#    log_boot("- Reading suicide result");
#    file_to_string(SUICIDE_DONE_FILE, suicide_done);
#    log_boot("- Reading help");
#    file_to_string(HELP_PAGE_FILE, help);

#    log_boot("- Loading rent mode");
#    log_boot("- Loading player list");
#    log_boot("- Loading reboot frequency");
#    log_boot("- Loading help files");

#    log_boot("- Loading fight messages");
#    load_messages();
#    log_boot("- Loading social messages");
#    boot_social_messages();
#    log_boot("- Loading pose messages");
#    boot_pose_messages();

#load_db();
#log_boot("Opening WHO port.");
#init_whod(port);
#log_boot("Opening I3 connection.");
#i3_startup(FALSE, 3000, FALSE);
#log_boot("Entering game loop.");
#game_loop(s);
#i3_shutdown(0);
#close_sockets(s);
#close_whod();
#unload_db();
#if (diku_reboot) {
#    log_boot("Rebooting.");
#    return MUD_REBOOT;
#}
#log_boot("Normal termination of game.");
#return MUD_HALT;					       /* what's so great about HHGTTG, anyhow? */

until($done) {
    my $loop_start_time = time();
    $time_daemon->update;
    $weather_daemon->update;
    log_info "Tick! Hour: %d, Month: %d", $time_daemon->hour, $time_daemon->month;
    foreach my $sock ($comm->poll) {
        my $message = '';
        my $rv = -1;

        $rv = sysread($sock, $message, 4096, length $message) while $rv;
        log_info "Socket %s has object %s", $sock, ref $comm->object($sock) if ref $comm->object($sock);
        log_info "Socket %s said \"%s\"", $sock, $message if length $message > 0;
        log_info "Socket %s closed by remote end.", $sock if defined $rv and $rv == 0;
        log_info "Socket %s closed by user request.", $sock if $message =~ /^quit\b/i;
        $comm->close($sock) if defined $rv and $rv == 0;
        $comm->close($sock) if $message =~ /^quit\b/i;
        $done = 1 if $message =~ /^shutdown\b/i;
    }
    log_info "%d connections", scalar $comm->connections;
    my $loop_duration = time() - $loop_start_time;
    sleep( 0.2 - $loop_duration ) if $loop_duration < 0.2;
}

END { log_fatal "System Halted."; }

1;

