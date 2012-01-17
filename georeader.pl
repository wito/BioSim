#!/usr/bin/perl -w
# georeader

my $rader = 0;
my $kolonner = 0;
my @map;
my @ini;
my $basename;
my $filename;

my $basedir;


#skriver html-filer for enkeltceller
sub writeCellXHTML {
  my $x = shift;
  my $y = shift;
  my $cell = <<EOF ;
<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1 plus MathML 2.0 plus SVG 1.1//EN"
"http://www.w3.org/2002/04/xhtml-math-svg/xhtml-math-svg.dtd">
<html>
  <head>
    <title>$basename : ($x,$y)</title>
    <link rel="stylesheet" href="BioSim.css" type="text/css" />
  </head>
  <body>
    <div id="header">
      <object type="image/svg+xml" data="$basename.svg" height="${\($rader*10)}px" width="${\($kolonner*10)}px">
        Geografi
      </object>

      <h1><a href="$basename.html">$basename</a> : ($x,$y)</h1>
    </div>
    <div id="footer">
      Boom!
    </div>
  </body>
</html>
EOF

open (XHTML,">$basedir/$basename.$x.$y.html");
print XHTML $cell;
close (XHTML);

}

#lager kart-innholdet
sub makeMap {
  my $rader = shift;
  my $kolonner = shift;
  my @map = @_;
  my @retval;
  my $flatmap = "";
  foreach (@map) {
    $flatmap.=$_;
  }
  for ($y=0;$y < $rader;$y++) {
    for ($x=0;$x < $kolonner;$x++) {
      $flatmap =~ s/(^.)//;
      my $thiscell = $1;
      push @retval,"    <a xlink:href='$basename.$x.$y.html' target='_top'>\n  "."    <rect class='$thiscell\' x='${\(($x)*10)}' y='${\(($y)*10)}' width='10' height='10' />\n"."    </a>\n";
      writeCellXHTML($x,$y);
    }
  }
  return @retval;
}

#leser innholdet i en BioSim-stil-inifil
sub readIni {
  my $file = shift;
  open(INIFILE,"<$file");
  while (<INIFILE>) {
    next if /^[#%]/;
    next if /^\s*$/;
    /^(\w+)\s+(.+)$/;
    $ini{$1} = $2;
  }
  close(INIFILE);
}

#lager en path for datGraph
sub makePath {
  my $x = 0;
  my $maxy = shift;
  my @data = @_;
  my $retval = "M";
  foreach (@data) {
    $retval.=($x*3)." ".($maxy-$_)." ";
    $x++;
  }
  return $retval;
}


#lager en svg-fil med grafer over populasjoner
sub datGraph {
  my @BJ;
  my @RJ;
  my @BS;
  my @RS;
  my @BO;
  my @RO;
  my $maxy = 0;
  my $maxx = 0;
  my $datafile = $ini{'UtdataStamme'};
  $datafile.=".dat";
  open(DATFILE,"<$datafile");
  while (<DATFILE>) {
    next if /^[#%]/;
    next if /^\s*$/;
    next if /^\w/;
    my @line = split /\s+/;
    my $maxan = 0;
    $maxx = $line[1];
    push @BJ, $line[2];
    push @RJ, $line[3];
    push @BS, $line[4];
    push @RS, $line[5];
    push @BO, $line[6];
    push @RO, $line[7];
    $maxan = $line[3] if $line[3] > $maxan;
    $maxan = $line[4] if $line[4] > $maxan;
    $maxan = $line[5] if $line[5] > $maxan;
    $maxan = $line[6] if $line[6] > $maxan;
    $maxan = $line[7] if $line[7] > $maxan;
    $maxy = $maxan if $maxan > $maxy;
  }
  close(DATFILE);
  
  my $BJ = makePath($maxy,@BJ);
  my $RJ = makePath($maxy,@RJ);
  my $RS = makePath($maxy,@BS);
  my $BS = makePath($maxy,@RS);
  my $RO = makePath($maxy,@BO);
  my $BO = makePath($maxy,@RO);
  
  my $graphdata = <<EOF ;
<?xml version="1.0" encoding="utf-8"?>
<?xml-stylesheet href="BioSim.css" type="text/css"?>
<!DOCTYPE svg PUBLIC "-//W3C//DTD SVG 1.1//EN"
"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd">
<svg xmlns='http://www.w3.org/2000/svg' xmlns:xlink="http://www.w3.org/1999/xlink" version="1.1" viewBox="0 0 ${\($maxx*3)} ${\($maxy)}" >
  <desc>
    Generated from $datafile.
  </desc>
  <g class="graph">
    <rect class="backdrop" x="0" y="0" width="100%" height="100%" />
    <path class="B J" d="$BJ" />
    <path class="R J" d="$RJ" />
    <path class="B S" d="$BS" />
    <path class="R S" d="$RS" />
    <path class="B O" d="$BO" />
    <path class="R O" d="$RO" />
  </g>
</svg>
EOF

  my $path = $ini{'UtdataStamme'}.'/Bjarnoya.graph.svg';

  open(GRAPHFILE,">$path");
  print GRAPHFILE $graphdata;
  close(GRAPHFILE);  
}

sub writeCSS {
  my $file = <<EOF;
object {
  float:right;
}

h1 {
  text-align:center;
}

div#footer {
  clear:both;
}

g.map rect {
  stroke:black;
  stroke-width:0.5;
}

g.map rect.H {
  fill:blue;
}

g.map rect.F {
  fill:gray;
}

g.map rect.S {
  fill:greenyellow;
}

g.map rect.O {
  fill:gold;
}

g.map rect.J {
  fill:green;
}

.year_data {
  display:none;
}

g.graph path {
  fill:none;
  stroke-width:3;
}

g.graph path.S {
  stroke:greenyellow;
}

g.graph path.O {
  stroke:gold;
}

g.graph path.J {
  stroke:green;
}

g.graph path.B {
  stroke-dasharray:1,1;
}

g.graph path.R {
  stroke-dasharray:2,3;
}

g.graph rect.backdrop {
}

g.graph .axis {
  stroke:black;
}
EOF

  open(CSSFILE,">$basedir/BioSim.css");
  print CSSFILE $file;
  close(CSSFILE);
}

#skriver hoved-xhtml-fila
sub writeBaseFile {
  my $file = <<EOF ;
<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1 plus MathML 2.0 plus SVG 1.1//EN"
"http://www.w3.org/2002/04/xhtml-math-svg/xhtml-math-svg.dtd">
<html>
  <head>
    <title>$basename</title>
    <link rel="stylesheet" href="BioSim.css" type="text/css" />
  </head>
  <body>
    <div id="header">
      <object type="image/svg+xml" data="$basename.svg" height="${\($rader*10)}px" width="${\($kolonner*10)}px">
        Geografi
      </object>

      <h1>$basename</h1>
    </div>
    <div id="footer">
      Boom!
    </div>
  </body>
</html>
EOF

open (BASEFILE,">$basedir/$basename.html");
print BASEFILE $file;
close (BASEFILE);
}

readIni($ARGV[0]);

$filename = $ini{'Geografi'}; #finner filnavnet
#$filename =~ s!.*/([^/]+)$!$1!; # hvorfor er denne kommentert ut?
$filename =~ m!/([^./]+)\.!; #finner navnestammen
$basename = $1;              #setter navnestammen

$basedir = $ini{'UtdataStamme'};  #finner utdatastammen
$basedir =~ s!/([^/])$!/html/$1!; #justerer samme.

mkdir ($basedir) unless -e $basedir; #oppretter utdatamappen om nødvendig.

#leser geografifila
open (GEOGRAFI,"<$filename");
while (<GEOGRAFI>) {
  next if /^%/; #hopper over kommentarer
  next if /^$/; #hopper over blanke linjer
  $rader = $1 if /^Rader\s+(\d+)/; #rader
  $kolonner = $1 if /^Kolonner\s+(\d+)/; #kolonner
  push @map, $1 if /^([A-Z]{$kolonner})$/; #kartdata
}
close (GEOGRAFI);

#svg-fil-base
$map = <<EOF ;
<?xml version="1.0" encoding="utf-8"?>
<?xml-stylesheet href="BioSim.css" type="text/css"?>
<!DOCTYPE svg PUBLIC "-//W3C//DTD SVG 1.1//EN" "http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd">
<svg xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" version="1.1" viewBox="0 0 ${\(($kolonner)*10)} ${\(($rader)*10)}">
  <desc>
    Generated from $filename.
  </desc>
  <g class="map">
%\%MAPGOESHERE%%  </g>
</svg>
EOF
#lager kartdata
@MAP = makeMap($rader,$kolonner,@map);
$MAP = "";
foreach (@MAP) {
  $MAP.=$_;
}

datGraph(); #hvorfor skjer dette her?
#setter inn kartdata
$map =~ s/%%MAPGOESHERE%%/$MAP/g;


#skriver ut filer
open (GEOSVG, ">$basedir/$basename.svg");
print GEOSVG $map;
close (GEOSVG);

writeBaseFile();
writeCSS();
