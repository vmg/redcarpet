require 'mkmf'

$CFLAGS << ' -fvisibility=hidden'

dir_config('greenmat')
create_makefile('greenmat')
