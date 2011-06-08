# encoding: utf-8
rootdir = File.dirname(File.dirname(__FILE__))
$LOAD_PATH.unshift "#{rootdir}/lib"

require 'test/unit'
require 'cgi'
require 'redcarpet'

class RedcarpetAutolinkTest < Test::Unit::TestCase
  def assert_linked(expected, url)
    assert_equal expected, Redcarpet.auto_link(url)
  end

  def test_autolink_works
    url = "http://example.com/"
    assert_linked "<a href=\"#{url}\">#{url}</a>", url
  end

  def test_not_autolink_www
    assert_linked "Awww... man", "Awww... man"
  end

  def test_does_not_terminate_on_dash
    url = "http://example.com/Notification_Center-GitHub-20101108-140050.jpg"
    assert_linked "<a href=\"#{url}\">#{url}</a>", url
  end

  def test_does_not_include_trailing_gt
    url = "http://example.com"
    assert_linked "&lt;<a href=\"#{url}\">#{url}</a>&gt;", "&lt;#{url}&gt;"
  end

  def test_links_with_anchors
    url = "https://github.com/github/hubot/blob/master/scripts/cream.js#L20-20"
    assert_linked "<a href=\"#{url}\">#{url}</a>", url
  end

  def test_links_like_rails
    urls = %w(http://www.rubyonrails.com
              http://www.rubyonrails.com:80
              http://www.rubyonrails.com/~minam
              https://www.rubyonrails.com/~minam
              http://www.rubyonrails.com/~minam/url%20with%20spaces
              http://www.rubyonrails.com/foo.cgi?something=here
              http://www.rubyonrails.com/foo.cgi?something=here&and=here
              http://www.rubyonrails.com/contact;new
              http://www.rubyonrails.com/contact;new%20with%20spaces
              http://www.rubyonrails.com/contact;new?with=query&string=params
              http://www.rubyonrails.com/~minam/contact;new?with=query&string=params
              http://en.wikipedia.org/wiki/Wikipedia:Today%27s_featured_picture_%28animation%29/January_20%2C_2007
              http://www.mail-archive.com/rails@lists.rubyonrails.org/
              http://www.amazon.com/Testing-Equal-Sign-In-Path/ref=pd_bbs_sr_1?ie=UTF8&s=books&qid=1198861734&sr=8-1
              http://en.wikipedia.org/wiki/Sprite_(computer_graphics)
              http://en.wikipedia.org/wiki/Texas_hold'em
              https://www.google.com/doku.php?id=gps:resource:scs:start
            )

    urls.each do |url|
      assert_linked %(<a href="#{url}">#{CGI.escapeHTML(url)}</a>), url
    end
  end

  def test_links_like_autolink_rails
    email_raw    = 'david@loudthinking.com'
    email_result = %{<a href="mailto:#{email_raw}">#{email_raw}</a>}
    email2_raw    = '+david@loudthinking.com'
    email2_result = %{<a href="mailto:#{email2_raw}">#{email2_raw}</a>}
    link_raw     = 'http://www.rubyonrails.com'
    link_result  = %{<a href="#{link_raw}">#{link_raw}</a>}
    link_result_with_options  = %{<a href="#{link_raw}" target="_blank">#{link_raw}</a>}
    link2_raw    = 'www.rubyonrails.com'
    link2_result = %{<a href="http://#{link2_raw}">#{link2_raw}</a>}
    link3_raw    = 'http://manuals.ruby-on-rails.com/read/chapter.need_a-period/103#page281'
    link3_result = %{<a href="#{link3_raw}">#{link3_raw}</a>}
    link4_raw    = 'http://foo.example.com/controller/action?parm=value&p2=v2#anchor123'
    link4_result = %{<a href="#{link4_raw}">#{CGI.escapeHTML(link4_raw)}</a>}
    link5_raw    = 'http://foo.example.com:3000/controller/action'
    link5_result = %{<a href="#{link5_raw}">#{link5_raw}</a>}
    link6_raw    = 'http://foo.example.com:3000/controller/action+pack'
    link6_result = %{<a href="#{link6_raw}">#{link6_raw}</a>}
    link7_raw    = 'http://foo.example.com/controller/action?parm=value&p2=v2#anchor-123'
    link7_result = %{<a href="#{link7_raw}">#{CGI.escapeHTML(link7_raw)}</a>}
    link8_raw    = 'http://foo.example.com:3000/controller/action.html'
    link8_result = %{<a href="#{link8_raw}">#{link8_raw}</a>}
    link9_raw    = 'http://business.timesonline.co.uk/article/0,,9065-2473189,00.html'
    link9_result = %{<a href="#{link9_raw}">#{link9_raw}</a>}
    link10_raw    = 'http://www.mail-archive.com/ruby-talk@ruby-lang.org/'
    link10_result = %{<a href="#{link10_raw}">#{link10_raw}</a>}

    assert_linked %(Go to #{link_result} and say hello to #{email_result}), "Go to #{link_raw} and say hello to #{email_raw}"
    assert_linked %(<p>Link #{link_result}</p>), "<p>Link #{link_raw}</p>"
    assert_linked %(<p>#{link_result} Link</p>), "<p>#{link_raw} Link</p>"
    assert_linked %(Go to #{link_result}.), %(Go to #{link_raw}.)
    assert_linked %(<p>Go to #{link_result}, then say hello to #{email_result}.</p>), %(<p>Go to #{link_raw}, then say hello to #{email_raw}.</p>)
    assert_linked %(<p>Link #{link2_result}</p>), "<p>Link #{link2_raw}</p>"
    assert_linked %(<p>#{link2_result} Link</p>), "<p>#{link2_raw} Link</p>"
    assert_linked %(Go to #{link2_result}.), %(Go to #{link2_raw}.)
#    assert_linked %(<p>Say hello to #{email_result}, then go to #{link2_result},</p>), %(<p>Say hello to #{email_raw}, then go to #{link2_raw},</p>)
    assert_linked %(<p>Link #{link3_result}</p>), "<p>Link #{link3_raw}</p>"
    assert_linked %(<p>#{link3_result} Link</p>), "<p>#{link3_raw} Link</p>"
    assert_linked %(Go to #{link3_result}.), %(Go to #{link3_raw}.)
    assert_linked %(<p>Go to #{link3_result}. seriously, #{link3_result}? i think I'll say hello to #{email_result}. instead.</p>), %(<p>Go to #{link3_raw}. seriously, #{link3_raw}? i think I'll say hello to #{email_raw}. instead.</p>)
    assert_linked %(<p>Link #{link4_result}</p>), "<p>Link #{link4_raw}</p>"
    assert_linked %(<p>#{link4_result} Link</p>), "<p>#{link4_raw} Link</p>"
    assert_linked %(<p>#{link5_result} Link</p>), "<p>#{link5_raw} Link</p>"
    assert_linked %(<p>#{link6_result} Link</p>), "<p>#{link6_raw} Link</p>"
    assert_linked %(<p>#{link7_result} Link</p>), "<p>#{link7_raw} Link</p>"
    assert_linked %(<p>Link #{link8_result}</p>), "<p>Link #{link8_raw}</p>"
    assert_linked %(<p>#{link8_result} Link</p>), "<p>#{link8_raw} Link</p>"
    assert_linked %(Go to #{link8_result}.), %(Go to #{link8_raw}.)
    assert_linked %(<p>Go to #{link8_result}. seriously, #{link8_result}? i think I'll say hello to #{email_result}. instead.</p>), %(<p>Go to #{link8_raw}. seriously, #{link8_raw}? i think I'll say hello to #{email_raw}. instead.</p>)
    assert_linked %(<p>Link #{link9_result}</p>), "<p>Link #{link9_raw}</p>"
    assert_linked %(<p>#{link9_result} Link</p>), "<p>#{link9_raw} Link</p>"
    assert_linked %(Go to #{link9_result}.), %(Go to #{link9_raw}.)
    assert_linked %(<p>Go to #{link9_result}. seriously, #{link9_result}? i think I'll say hello to #{email_result}. instead.</p>), %(<p>Go to #{link9_raw}. seriously, #{link9_raw}? i think I'll say hello to #{email_raw}. instead.</p>)
    assert_linked %(<p>#{link10_result} Link</p>), "<p>#{link10_raw} Link</p>"
    assert_linked email2_result, email2_raw
    assert_linked "#{link_result} #{link_result} #{link_result}", "#{link_raw} #{link_raw} #{link_raw}"
    assert_linked '<a href="http://www.rubyonrails.com">Ruby On Rails</a>', '<a href="http://www.rubyonrails.com">Ruby On Rails</a>'
  end
end
