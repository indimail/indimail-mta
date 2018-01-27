package mkcommon;

use 5.012002;
use strict;
use warnings;

require Exporter;

our @ISA = qw(Exporter);

# Items to export into callers namespace by default. Note: do not export
# names by default without a very good reason. Use EXPORT_OK instead.
# Do not simply export all your public functions/methods/constants.

# This allows declaration	use mkcommon ':all';
# If you do not need this, moving things directly into @EXPORT or @EXPORT_OK
# will save memory.
our %EXPORT_TAGS = ( 'all' => [ qw(

) ] );

our @EXPORT_OK = ( @{ $EXPORT_TAGS{'all'} } );

our @EXPORT = qw(

);

our $VERSION = '0.01';

my $BLOCK_SIZE=256;

# Preloaded methods go here.

sub new {

    my ($this, %params) = @_;

    my $class = ref($this) || $this;
    my $self = \%params;

    bless $self, $class;

    $self->{'char_array'}=[];
    $self->{'char_class'}=[];
    $self->{'char_start'}=[0];

    $self->{'last_block'}=-1;
    $self->{'last'}="";
    $self->{'last_f'}=0;
    $self->{'last_l'}=0;

    $self->{"classtype"} //= "uint8_t";

    return $self;
}

sub _doemit_block {
    my $this=shift;

    my $f=shift;
    my $l=shift;

    push @{$this->{'char_array'}}, [$f, $l];
    push @{$this->{'char_class'}}, $this->{'last'};
}

sub _doemit_endblock {

    my $this=shift;

    push @{$this->{'char_start'}}, $#{$this->{'char_array'}}+1;
}

# _doemit invokes _doemit_block() for each unicode char range with a given
# linebreaking class. However, once a unicode char range starts in a different
# $BLOCK_SIZE character class, call _doemit_endblock() before calling _doemit_block().
#
# If a single unicode char range crosses a $BLOCK_SIZE character class boundary,
# split it at the boundary; call _doemit_endblock() to finish the current $BLOCK_SIZE
# char boundary, call _doemit_endblock(), then call _doemit_block() for the
# rest of the char range.


sub _doemit {

    my $this=shift;

    $this->_doemit_endblock()
	if int($this->{'last_f'} / $BLOCK_SIZE)
	!= $this->{'last_block'} && $this->{'last_block'} != -1;

    if (int($this->{'last_f'} / $BLOCK_SIZE) != int($this->{'last_l'} / $BLOCK_SIZE))
    {
	while (int($this->{'last_f'} / $BLOCK_SIZE) != int($this->{'last_l'} / $BLOCK_SIZE))
	{
	    my $n=int($this->{'last_f'} / $BLOCK_SIZE) * $BLOCK_SIZE + ($BLOCK_SIZE-1);

	    $this->_doemit_block($this->{'last_f'}, $n);
	    $this->_doemit_endblock();
	    $this->{'last_f'}=$n+1;
	}
    }
    $this->_doemit_block($this->{'last_f'}, $this->{'last_l'});

    $this->{'last_block'}=int($this->{'last_l'} / $BLOCK_SIZE);
}

#
# Coalesce adjacent unicode char blocks that have the same linebreaking
# property. Invoke _doemit() for the accumulate unicode char range once
# a range with a different linebreaking class is seen.

sub range {

    my $this=shift;

    my $f=shift;
    my $l=shift;
    my $t=shift;

    if ($this->{'last_l'} + 1 == $f && $this->{'last'} eq $t)
    {
	$this->{'last_l'}=$l;
	return;
    }

    $this->_doemit() if $this->{'last'};  # New linebreaking class

    $this->{'last_f'}=$f;
    $this->{'last_l'}=$l;
    $this->{'last'}=$t;
}

sub output {
    my $this=shift;

    $this->_doemit();  # Emit last linebreaking unicode char range class

    $this->_doemit_endblock(); # End of the most recent $BLOCK_SIZE char range class

    print "static const uint8_t unicode_rangetab[][2]={\n";

    my $comma="\t";

    my $modulo=sprintf("0x%X", $BLOCK_SIZE-1);

    foreach ( @{$this->{'char_array'}} )
    {
	print "${comma}{0x" . sprintf("%04x", $$_[0]) . " & $modulo, 0x"
	    . sprintf("%04x", $$_[1]) . " & $modulo}";
	$comma=",\n\t";
    }

    print "};\n\n";

    print "static const " . $this->{classtype} . " unicode_classtab[]={\n";

    $comma="\t";
    foreach ( @{$this->{'char_class'}} )
    {
	print "${comma}$_";
	$comma=",\n\t";
    }

    print "};\n\n";

    print "static const size_t unicode_indextab[]={\n";

    $comma="\t";

    my $prev_block=-1;
    foreach (@{$this->{'char_start'}})
    {
	my $sp=$_;
	my $cnt=1;

	if ($sp <= $#{$this->{'char_array'}})
	{
	    my $block=int($this->{'char_array'}->[$sp]->[0] / $BLOCK_SIZE);

	    $cnt = $block - $prev_block;
	    $prev_block=$block;
	}

	foreach (1..$cnt)
	{
	    print "$comma$sp";
	    $comma=",\n\t";
	}
    }

    print "};\n\n";
}

1;
