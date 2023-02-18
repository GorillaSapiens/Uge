#!/usr/bin/env perl

@x = `echo 2^128 | bc`;
$x = $x[0];
$x =~ s/[\x0a\x0d]//g;
$l = length($x);

$l += 5; # arbitrary!

print "#define LENGTH128 $l\n";
print "// values below are in reverse!\n";

for ($i = 0; $i < 128; $i++) {
   @x = `echo 2^$i | bc`;
   $x = $x[0];
   $x =~ s/[\x0a\x0d]//g;
   $x = reverse($x);
   $x = "\"$x\", ";
   $x .= "// 2^$i\n";

   print $x;
}
