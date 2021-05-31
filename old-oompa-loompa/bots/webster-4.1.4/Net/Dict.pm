#
# Net::Dict.pm
#
# Copyright (C) 2001 Neil Bowers <neilb@cre.canon.co.uk>
# Copyright (c) 1998 Dmitry Rubinstein <dimrub@wisdom.weizmann.ac.il>.
#
# All rights reserved.  This program is free software; you can
# redistribute it and/or modify it under the same terms as Perl
# itself.
#
# $Id: Dict.pm,v 2.4 2001/04/23 19:37:07 neilb Exp $
#

package Net::Dict;

use strict;
use IO::Socket;
use Net::Cmd;
use Carp;

use vars qw(@ISA $VERSION $debug);
$VERSION = sprintf("%d.%02d", q$Revision: 2.4 $ =~ /(\d+)\.(\d+)/);

#-----------------------------------------------------------------------
# Default values for arguments to new(). We also use this to
# determine valid argument names - if it's not a key of this hash,
# then it's not a valid argument.
#-----------------------------------------------------------------------
my %ARG_DEFAULT =
(
 Port    => 2628,
 Timeout => 120,
 Debug   => 0,
 Client  => "Net::Dict v$VERSION",
);

@ISA = qw(Net::Cmd IO::Socket::INET);

#=======================================================================
#
# new()
#
# constructor - open connection to host, get a list of databases,
# and send CLIENT identification command.
#
#=======================================================================
sub new
{
    @_ > 1 or croak 'usage: Net::Dict->new() takes at least a HOST name';
    my $class  = shift;
    my $host   = shift;
    int(@_) % 2 == 0 or croak 'Net::Dict->new(): odd number of arguments';
    my %inargs = @_;

    my $self;
    my $argref;


    return undef unless defined $host;

    #-------------------------------------------------------------------
    # Process arguments, setting defaults if needed
    #-------------------------------------------------------------------
    $argref = {};
    foreach my $arg (keys %ARG_DEFAULT)
    {
        $argref->{$arg} = exists $inargs{$arg}
                          ? $inargs{$arg}
                          : $ARG_DEFAULT{$arg};
        delete $inargs{$arg};
    }
    if (keys(%inargs) > 0)
    {
        croak "Net::Dict->new(): unknown argument - ",
            join(', ', keys %inargs);
    }

    #-------------------------------------------------------------------
    # Make the connection
    #-------------------------------------------------------------------
    $self = $class->SUPER::new(PeerAddr => $host,
                               PeerPort => $argref->{Port},
                               Proto    => 'tcp',
                               Timeout  => $argref->{Timeout}
                               );

    return undef
	unless defined $self;

    ${*$self}{'net_dict_host'} = $host;

    $self->autoflush(1);
    $self->debug($argref->{Debug});

    if ($self->response() != CMD_OK)
    {
        $self->close();
        return undef;
    }

    # parse the initial 220 response
    $self->_parse_banner($self->message);

    #-------------------------------------------------------------------
    # Send the CLIENT command which identifies the connecting client
    #-------------------------------------------------------------------
    $self->_CLIENT($argref->{Client});

    #-------------------------------------------------------------------
    # The default - search ALL dictionaries
    #-------------------------------------------------------------------
    $self->setDicts('*');

    return $self;
}

sub dbs
{
    @_ == 1 or croak 'usage: $dict->dbs() - takes no arguments';
    my $self = shift;


    $self->_get_database_list();
    return %{${*$self}{'net_dict_dbs'}};
}

sub setDicts
{
    my $self = shift;

    @{${*$self}{'net_dict_userdbs'}} = @_;
}

sub serverInfo
{
    @_ == 1 or croak 'usage: $dict->serverInfo()';
    my $self = shift;

    return 0
        unless $self->_SHOW_SERVER();
    my $info = join('', @{$self->read_until_dot});
    $self->getline();
    $info;
}

sub dbInfo
{
    @_ == 2 or croak 'usage: $dict->dbInfo($dbname) - one argument only';
    my $self = shift;

    if ($self->_SHOW_INFO(@_))
    {
        return join('', @{$self->read_until_dot()});
    }
    else
    {
        return undef;
    }
}

sub dbTitle
{
    @_ == 2 or croak 'dbTitle() method expects one argument - DB name';
    my $self   = shift;
    my $dbname = shift;


    $self->_get_database_list();
    if (exists ${${*$self}{'net_dict_dbs'}}{$dbname})
    {
        return ${${*$self}{'net_dict_dbs'}}{$dbname};
    }
    else
    {
        carp 'dbTitle(): unknown database name' if $self->debug;
        return undef;
    }
}

sub strategies
{
    @_ == 1 or croak 'usage: $dict->strategies()';
    my $self = shift;
    return 0
        unless $self->_SHOW_STRAT();
    my(%strats, $name, $desc);
    foreach (@{$self->read_until_dot()})
    {
        ($name, $desc) = (split /\s/, $_, 2);
        chomp $desc;
        $strats{$name} = _unquote($desc);
    }
    $self->getline();
    %strats;
}

sub define
{
    @_ >= 2 or croak 'usage: $dict->define($word [, @dbs]) - takes at least one argument';
    my $self = shift;
    my $word = shift;
    my @dbs = (@_ > 0) ? @_ : @{${*$self}{'net_dict_userdbs'}};
    croak 'select some dictionaries with setDicts or supply as argument to define'
        unless @dbs;
    my($db, @defs);


    #-------------------------------------------------------------------
    # check whether we got an empty word
    #-------------------------------------------------------------------
    if (!defined($word) || $word eq '')
    {
        carp "empty word passed to define() method";
        return undef;
    }

    foreach $db (@dbs)
    {
        next
            unless $self->_DEFINE($db, $word);

        my ($defNum) = ($self->message =~ /^\d{3} (\d+) /);
        foreach (0..$defNum-1)
        {
            my ($d) = ($self->getline =~ /^\d{3} ".*" (\w+) /);
            my ($def) = join '', @{$self->read_until_dot};
            push @defs, [$d, $def];
        }
        $self->getline();
    }
    \@defs;
}

sub match
{
    @_ >= 3 or croak 'usage: $self->match($word, $strat [, @dbs]) - takes at least two arguments';
    my $self = shift;
    my $word = shift;
    my $strat = shift;
    my @dbs = (@_ > 0) ? @_ : @{${*$self}{'net_dict_userdbs'}};
    croak 'define some dictionaries by setDicts or supply as argument to define'
        unless @dbs;
    my ($db, @matches);

    #-------------------------------------------------------------------
    # check whether we got an empty pattern
    #-------------------------------------------------------------------
    if (!defined($word) || $word eq '')
    {
        carp "empty pattern passed to match() method";
        return undef;
    }

    foreach $db (@dbs)
    {
        next unless $self->_MATCH($db, $strat, $word);

        my ($db, $w);
        foreach (@{$self->read_until_dot}) {
            ($db, $w) = split /\s/, $_, 2;
            chomp $w;
            push @matches, [$db, _unquote($w)];
        }
        $self->getline();
    }
    \@matches; 
}

sub auth
{
    @_ == 3 or croak 'usage: $dict->auth() - takes two arguments';
    my $self        = shift;
    my $user        = shift;
    my $pass_phrase = shift;
    my $auth_string;
    my $string;
    my $ctx;


    require Digest::MD5;
    $string = $self->msg_id().$pass_phrase;
    $auth_string = Digest::MD5::md5_hex($string);

    if ($self->_AUTH($user, $auth_string))
    {
        #---------------------------------------------------------------
        # clear the cache of database names
        # next time a method needs them, this will cause us to go
        # back to the server, and thus pick up any AUTH-restricted DBs
        #---------------------------------------------------------------
        delete ${*$self}{'net_dict_dbs'};
    }
    else
    {
        carp "auth() failed with error code ".$self->code() if $self->debug();
        return;
    }
}

sub status
{
    @_ == 1 or croak 'usage: $dict->status() - takes no arguments';
    my $self = shift;
    my $message;


    $self->_STATUS() || return 0;
    chomp($message = $self->message);
    $message =~ s/^\d{3} //;
    return $message;
}

sub capabilities
{
    @_ == 1 or croak 'usage: $dict->capabilities() - takes no arguments';
    my $self = shift;


    return @{ ${*$self}{'net_dict_capabilities'} };
}

sub has_capability
{
    @_ == 2 or croak 'usage: $dict->has_capability() - takes one argument';
    my $self = shift;
    my $cap  = shift;


    return grep(lc($cap) eq $_, $self->capabilities());
}

sub msg_id
{
    @_ == 1 or croak 'usage: $dict->msg_id() - takes no arguments';
    my $self = shift;


    return ${*$self}{'net_dict_msgid'};
}


sub _DEFINE { shift->command('DEFINE', map { '"'.$_.'"' } @_)->response() == CMD_INFO }
sub _MATCH { shift->command('MATCH', map { '"'.$_.'"' } @_)->response() == CMD_INFO }
sub _SHOW_DB { shift->command('SHOW DB')->response() == CMD_INFO }
sub _SHOW_STRAT { shift->command('SHOW STRAT')->response() == CMD_INFO }
sub _SHOW_INFO { shift->command('SHOW INFO', @_)->response() == CMD_INFO }
sub _SHOW_SERVER { shift->command('SHOW SERVER')->response() == CMD_INFO }
sub _CLIENT { shift->command('CLIENT', @_)->response() == CMD_OK }
sub _STATUS { shift->command('STATUS')->response() == CMD_OK }
sub _HELP { shift->command('HELP')->response() == CMD_INFO }
sub _QUIT { shift->command('QUIT')->response() == CMD_OK }
sub _OPTION_MIME { shift->command('OPTION MIME')->response() == CMD_OK }
sub _AUTH { shift->command('AUTH', @_)->response() == CMD_OK }
sub _SASLAUTH { shift->command('SASLAUTH', @_)->response() == CMD_OK }
sub _SASLRESP { shift->command('SASLRESP', @_)->response() == CMD_OK }

sub quit
{
    my $self = shift;

    $self->_QUIT;
    $self->close;
}

sub DESTROY
{
    my $self = shift;

    if (defined fileno($self)) {
        $self->quit;
    }
}

sub response
{
    my $self = shift;
    my $str = $self->getline() || return undef;


    if ($self->debug)
    {
        $self->debug_print(0,$str);
    }

    my($code) = ($str =~ /^(\d+) /);

    ${*$self}{'net_cmd_resp'} = [ $str ];
    ${*$self}{'net_cmd_code'} = $code;

    substr($code,0,1);
}

#=======================================================================
#
# _unquote
#
# Private function used to remove quotation marks from around
# a string.
#
#=======================================================================
sub _unquote
{
    my $string = shift;


    if ($string =~ /^"/)
    {
        $string =~ s/^"//;
        $string =~ s/"$//;
    }
    return $string;
}

#=======================================================================
#
# _parse_banner
#
# Parse the initial response banner the server sends when we connect.
# Hoping for:
#      220 blah blah <auth.mime> <msgid>
# The <auth.mime> string gives a list of supported extensions.
# The last bit is a msg-id, which identifies this connection,
# and is used in authentication, for example.
#
#=======================================================================
sub _parse_banner
{
    my $self   = shift;
    my $banner = shift;
    my ($code, $capstring, $msgid);


    ${*$self}{'net_dict_banner'} = $banner;
    ${*$self}{'net_dict_capabilities'} = [];
    if ($banner =~ /^(\d{3}) (.*) (<[^<>]*>)?\s+(<[^<>]+>)\s*$/)
    {
        ${*$self}{'net_dict_msgid'} = $4;
        ($capstring = $3) =~ s/[<>]//g;
        if (length($capstring) > 0)
        {
            ${*$self}{'net_dict_capabilities'} = [split(/\./, $capstring)];
        }
    }
    else
    {
        carp "unexpected format for welcome banner on connection:\n",
             $banner if $self->debug;
    }
}

#=======================================================================
#
# _get_database_list
#
# Get the list of databases on the remote server.
# We cache them in the instance data object, so that dbTitle()
# and databases() don't have to go to the server every time.
#
# We check to see whether we've already got the databases first,
# and do nothing if so. This means that this private method
# can just be invoked in the public methods.
# 
#=======================================================================
sub _get_database_list
{
    my $self = shift;


    return if exists ${*$self}{'net_dict_dbs'};

    if ($self->_SHOW_DB)
    {
	my($dbNum)= ($self->message =~ /^\d{3} (\d+)/);
	my($name, $descr);
 	foreach (0..$dbNum-1)
        {
            ($name, $descr) = (split /\s/, $self->getline, 2);
            chomp $descr;
            ${${*$self}{'net_dict_dbs'}}{$name} = _unquote($descr);
	}
	# Is there a way to do it right? Reading the dot line and the
	# status line afterwards? Maybe I should use read_until_dot?
	$self->getline();
	$self->getline();
    }
}

#-----------------------------------------------------------------------
# Method aliases for backwards compatibility
#-----------------------------------------------------------------------
*strats = \&strategies;

1;

__END__

=head1 NAME

Net::Dict - client API for accessing dictionary servers (RFC 2229)

=head1 SYNOPSIS

    use Net::Dict;
    
    $dict = Net::Dict->new('dict.server.host');
    $h = $dict->define("word");
    foreach $i (@{$h}) {
        ($db, $def) = @{$i};
	. . .
    }

=head1 DESCRIPTION

C<Net::Dict> is a perl class for looking up words and their
definitions on network dictionary servers.
C<Net::Dict> provides a simple DICT client API for the network
protocol described in RFC2229. Quoting from that RFC:

=over

=item

The Dictionary Server Protocol (DICT) is a TCP transaction based
query/response protocol that allows a client to access dictionary
definitions from a set of natural language dictionary databases.

=back

An instance of Net::Dict represents a connection to a single
DICT server. For example, to connect to the dictionary
server at C<dict.org>, you would write:

    $dict = Net::Dict->new('dict.org');

A DICT server can provide any number of dictionaries,
which are referred to as I<databases>.
Each database has a I<name> and a I<title>.
The name is a short identifier,
typically just one word, used to refer to that database.
The title is a brief one-line description of the database.
For example, at the time of writing, the C<dict.org> server
has 11 databases, including a version of Webster's
dictionary from 1913. The name of the database is I<web1913>,
and the title is I<Webster's Revised Unabridged Dictionary (1913)>.

To look up definitions for a word, you use the C<define> method:

    $dref = $dict->define('banana');

This returns a reference to a list; each entry in the list
is a reference to a two item list:

    [ $dbname, $definition ]

The first entry is a I<database name> as introduced above.
The second entry is the text of a definition from
the specified dictionary.

=head2 MATCHING WORDS

In addition the looking up word definitions,
you can lookup a list of words which match a given
pattern, using the B<match()> method.
Each DICT server typically supports a number of I<strategies>
which can be used to match words against a pattern.
For example, using B<prefix> strategy with a pattern "anti"
would find all words in databases which start with "anti":

    @mref = $dict->match('anti', 'prefix');
    foreach my $match (@{ $mref })
    {
        ($db, $word) = @{ $match };
    }

Similarly the B<suffix> strategy is used to search for words
which end in a given pattern.
The B<strategies()> method is used to request a list of supported
strategies - see L<"METHODS"> for more details.

=head2 SELECTING DATABASES

By default Net::Dict will look in all databases on the DICT server.
This is specified with a special database name of C<*>.
You can specify the database(s) to search explicitly,
as additional arguments to the C<define> method:

    $dref = $dict->define('banana', 'wn', 'web1913');

Rather than specify the databases to use every time,
you can change the default from '*' using the C<setDicts> method:

    $dict->setDicts('wn', 'web1913');

Any subsequent calls to C<define> will refer to these databases,
unless over-ridden with additional arguments to C<define>.
You can find out what databases are available on a server
using the C<dbs> method:

    %dbhash = $dict->dbs();

Each entry in the returned hash has the name of a database as the key,
and the corresponding title as the value.

There is another special database name - C<!> - which says that
all databases should be searched, but as soon as a definition is
found, no further databases should be searched.

=head1 CONSTRUCTOR

    $dict = Net::Dict->new (HOST [,OPTIONS]);

This is the constructor for a new Net::Dict object. C<HOST> is the
name of the remote host on which a Dict server is running.
This is required, and must be an explicit host name.

B<Note:> previous versions let you give an empty string
for the hostname, resulting in selection of default hosts.
This behaviour is no longer supported.

C<OPTIONS> are passed in a hash like fashion, using key and value pairs.
Possible options are:

=over 4

=item B<Port>

The port number to connect to on the remote machine for the
Dict connection (a default port number is 2628, according to RFC2229).

=item B<Client>

The string to send as the CLIENT identifier.
If not set, then a default identifier for Net::Dict is sent.

=item B<Timeout>

Sets the timeout for the connection, in seconds.
Defaults to 120.

=item B<Debug>

The debug level - a non-zero value will resulting in debugging
information being generated, particularly when errors occur.
Can be changed later using the C<debug> method,
which is inherited from Net::Cmd.
More on the debug method can be found in L<Net::Cmd>.

=back

Making everything explicit, here's how you might call
the constructor in your client:

    $dict = Net::Dict->new($HOST,
                           Port    => 2628,
                           Client  => "myclient v$VERSION",
                           Timeout => 120,
                           Debug   => 0);

This will return C<undef> if we failed to make the connection.
It will C<die> if bad arguments are passed: no hostname,
unknown argument, etc.

=head1 METHODS

Unless otherwise stated all methods return either a I<true> or I<false>
value, with I<true> meaning that the operation was a success. When a method
states that it returns a value, failure will be returned as I<undef> or an
empty list.


=head2 define ( $word [, @dbs] )

returns a reference to an array, whose members are lists,
consisting of two elements: the dictionary name and the definition.
If no dictionaries are specified, those set by setDicts() are used.


=head2 match ( $pattern, $strategy [, @dbs] )

Looks for words which match $pattern according to the specified
matching $strategy.
Returns a reference to an array,
each entry of which is a reference to a two-element
array: database name, matching word.

=head2 dbs

Returns a hash with information on the databases available
on the DICT server.
The keys are the short names, or identifiers, of the databases;
the value is title of the database:

    %dbhash = $dict->dbs();
    print "Available dictionaries:\n";
    while (($db, $title) = each %dbhash)
    {
        print "$db : $title\n";
    }

This is the C<SHOW DATABASES> command from RFC 2229.


=head2 dbInfo ( $dbname )

Returns a string, containing description of
the dictionary $dbname. 


=head2 setDicts ( @dicts )

Specify the dictionaries that will be
searched during the successive define() or match() calls.
Defaults to '*'.
No existance checks are performed by this interface, so you'd better make
sure the dictionaries you specify are on the server (e.g. by calling
dbs()).


=head2 strategies

returns an array, containing an ID of a matching strategy
as a key and a verbose description as a value.

This method was previously called strats();
that name for the method is also currently supported,
for backwards compatibility.

=head2 auth ( $USER, $PASSPHRASE )

Attempt to authenticate the specified user, using the scheme
described on page 18 of RFC 2229.
The user should be known to the server, and $PASSPHRASE
is a shared secret known only to the server and the user.

For example, if you were using dictd from dict.org,
your configuration file might include the following:

    database private {
        data "/usr/local/dictd/db/private.dict.dz"
        index "/usr/local/dictd/db/private.index"
        access { user connor }
    }

    user connor "there can be only one"

To be able to access this database, you'd write
something like the following:

    $dict = Net::Dict->new('dict.foobar.com');
    $dict->auth('connor', 'there can be only one');

A subsequent call to the C<databases> method would
reveal the C<private> database now accessible.
Not all servers support the AUTH extension;
you can check this with the has_capability() method,
described below.


=head2 serverInfo

Returns a string, containing the information about the server,
provided by the server:

    print "Server Info:\n";
    print $dict->serverInfo(), "\n";

This is the C<SHOW SERVER> command from RFC 2229.


=head2 dbTitle ( $DBNAME )

Returns the title string for the specified database.
This is the same string returned by the C<dbs()> method
for all databases.

=head2 capabilities

Returns a list of the capabilities supported by the DICT server,
as described on pages 7 and 8 of RFC 2229.

=head2 has_capability ( $cap_name )

Returns true (non-zero) if the DICT server supports the
specified capability; false (zero) otherwise. Eg

    if ($dict->has_capability('auth')) {
        $dict->auth('genie', 'open sesame');
    }

=head2 status

Send the STATUS command to the DICT server,
which will return some server-specific timing
or debugging information.
This may be useful when debugging or tuning a DICT server,
but probably won't be of interest to most users.


=head1 KNOWN BUGS AND LIMITATIONS

=over 4

=item *

The following DICT commands are not currently supported:

    OPTION MIME

=item *

No support for firewalls at the moment.

=item *

Site-wide configuration isn't supported. Previous documentation
suggested that it was.

=item *

Currently no way to specify that results of define and match
should be in HTML. This was also previously a config option
for the constructor, but it didn't do anything.

=back

=head1 REPORTING BUGS

When reporting bugs/problems please include as much information as possible.
It may be difficult for me to reproduce the problem as almost every setup
is different.

A small script which yields the problem will probably be of help. It would
also be useful if this script was run with the extra options C<Debug =E<gt> 1>
passed to the constructor, and the output sent with the bug report. If you
cannot include a small script then please include a Debug trace from a
run of your program which does yield the problem.

=head1 EXAMPLES

The B<examples> directory of the Net-Dict distribution
includes C<simple.pl>, which illustrates basic use of the module.

The distribution also includes two example DICT clients:
B<dict> is a basic command-line client, and B<tkdict>
is a GUI-based client, created using Perl/Tk.

=head1 SEE ALSO

=over 4

=item RFC 2229

The internet document which defines the DICT protocol.

http://www.cis.ohio-state.edu/htbin/rfc/rfc2229.html

=item Net::Cmd

A module which provides methods for a network command class,
such as Net::FTP, Net::SMTP, as well as Net::Dict.
Part of the libnet distribution, available from CPAN.

=item dictd(8)

The reference DICT server, available from B<dict.org>.

=item dict(1)

The sample client, written in C, which comes with dictd.

=item http://www.dict.org/

The home page for the DICT effort; has links to other resources,
including other libraries and clients.

=back

=head1 AUTHOR

Net::Dict was written by
Dmitry Rubinstein E<lt>dimrub@wisdom.weizmann.ac.ilE<gt>,
using Net::FTP and Net::SMTP as a pattern and a model for imitation.

The module is now maintained by
Neil Bowers E<lt>neilb@cre.canon.co.ukE<gt>

=head1 COPYRIGHT

Copyright (C) 2001 Canon Research Centre Europe, Ltd.

Copyright (c) 1998 Dmitry Rubinstein. All rights reserved.

This module is free software; you can redistribute it and/or modify it
under the same terms as Perl itself.

=cut

