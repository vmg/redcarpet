require 'mkmf'

$CFLAGS << ' -fvisibility=hidden'

# Check for GC compaction support (Ruby 2.7+)
have_func('rb_gc_location')

dir_config('redcarpet')
create_makefile('redcarpet')
