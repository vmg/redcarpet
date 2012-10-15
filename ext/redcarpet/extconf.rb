require 'mkmf'

dir_config('redcarpet')
create_makefile('redcarpet')

have_header 'ruby/st.h' or have_header 'st.h' or abort "redcarpet requires the ruby/st.h header"
