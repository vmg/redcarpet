desc 'Gather required Sundown sources into extension directory'
task :gather => 'sundown:checkout' do |t|
  files =
    FileList[
      'sundown/src/{markdown,buffer,stack,autolink,html_blocks}.h',
      'sundown/src/{markdown,buffer,stack,autolink}.c',
      'sundown/html/{html,html_smartypants,houdini_html_e,houdini_href_e}.c',
      'sundown/html/{html,houdini}.h',
    ]
  cp files, 'ext/redcarpet/',
    :preserve => true,
    :verbose => true
end

desc 'Checkout last Sundown sources'
task 'sundown:checkout' do |t|
  unless File.exists?('sundown/src/markdown.h')
    sh 'git submodule init'
    sh 'git submodule update'
  end
end
