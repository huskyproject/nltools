# BeOS sample configuration for the nodelist tools

# use "export FIDOCONFIG=/path/to/this/config.file" in the batch that
# calls the tools, or in /boot/home/.profile

# === Generic settings ===

# Make sure these unpackers are in your search path, esp. ARC 5.21!
Unpack "/bin/unzip -joLqq $a -d $p" 0 504b0304 
# Get unrar for BeOS at: www.bebits.com/app/414
Unpack "/boot/home/config/bin/unrar e -o+ -y -c- -p- $a $p/ >/dev/null" 0 52617221
# Get arc for BeOS at: www.physcip.uni-stuttgart.de/tobi7bin/arc521be.zip
Unpack "/boot/home/config/bin/arc eno $a $p'/*.*'" 0 1a


# Directory in which the nltools.log file will be stored:
logFileDir /boot/home/fido/log

# Where the nodelist is and where the fidouser.lst goes:
nodelistDir /boot/home/fido/nodelist

# Point this to any existing directory, the nltools do not use it:
msgBaseDir /boot/home/fido/msgbase

# Filename of fidouser list (without path!)
fidoUserList fidouser.lst

# === Fidonet Worldwide Nodelist ===

# nodelist base file name
Nodelist nodelist

# where nodediffs can be found (path + base filename)
DiffUpdate /boot/home/fido/filebase/nodediff/nodediff


# where full updates can be found, if any (path + base filename)
FullUpdate /boot/home/fido/filebase/nodelist/nodelist

# format of this nodelist
NodelistFormat Standard

# === German R24 Fidonet Point list ===

# nodelist base file name
Nodelist points24

# where nodediffs can be found (path + base filename)
DiffUpdate /boot/home/fido/filebase/24000/pr24diff

# format of this nodelist
Nodelistformat Points24

# Points24 requires a default zone!
DefaultZone 2


