# encoding: utf-8
rootdir = File.dirname(File.dirname(__FILE__))
$LOAD_PATH.unshift "#{rootdir}/lib"

require 'test/unit'
require 'cgi'
require 'redcarpet'

class RedcarpetRailsTest < Test::Unit::TestCase
  def assert_linked(expected, url)
    assert_equal "<p>#{expected}</p>", Redcarpet.new(url, :autolink).to_html.strip
  end

  def test_auto_link_works
    url = "http://example.com/"
    assert_linked "<a href=\"#{url}\">#{url}</a>", url
  end

  def test_does_not_autolink_www
    assert_linked "Awww... man", "Awww... man"
  end

  def test_does_not_terminate_on_dash
    url = "http://example.com/Notification_Center-GitHub-20101108-140050.jpg"
    assert_linked "<a href=\"#{url}\">#{url}</a>", url
  end

  def test_does_not_include_gt
    url = "http://example.com/"
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

  def test_like_rails_rules
    email_raw    = 'david@loudthinking.com'
    email_result = %{<a href="mailto:#{email_raw}">#{email_raw}</a>}
    email2_raw    = '+david@loudthinking.com'
    email2_result = %{<a href="mailto:#{email2_raw}">#{email2_raw}</a>}
    link_raw     = 'http://www.rubyonrails.com'
    link_result  = %{<a href="#{link_raw}">#{link_raw}</a>}
    link_result_with_options  = %{<a href="#{link_raw}" target="_blank">#{CGI.escapeHTML(link_raw)}</a>}
    link2_raw    = 'www.rubyonrails.com'
    link2_result = %{<a href="http://#{link2_raw}">#{CGI.escapeHTML(link2_raw)}</a>}
    link3_raw    = 'http://manuals.ruby-on-rails.com/read/chapter.need_a-period/103#page281'
    link3_result = %{<a href="#{link3_raw}">#{CGI.escapeHTML(link3_raw)}</a>}
    link4_raw    = 'http://foo.example.com/controller/action?parm=value&p2=v2#anchor123'
    link4_result = %{<a href="#{link4_raw}">#{CGI.escapeHTML(link4_raw)}</a>}
    link5_raw    = 'http://foo.example.com:3000/controller/action'
    link5_result = %{<a href="#{link5_raw}">#{CGI.escapeHTML(link5_raw)}</a>}
    link6_raw    = 'http://foo.example.com:3000/controller/action+pack'
    link6_result = %{<a href="#{link6_raw}">#{CGI.escapeHTML(link6_raw)}</a>}
    link7_raw    = 'http://foo.example.com/controller/action?parm=value&p2=v2#anchor-123'
    link7_result = %{<a href="#{link7_raw}">#{CGI.escapeHTML(link7_raw)}</a>}
    link8_raw    = 'http://foo.example.com:3000/controller/action.html'
    link8_result = %{<a href="#{link8_raw}">#{CGI.escapeHTML(link8_raw)}</a>}
    link9_raw    = 'http://business.timesonline.co.uk/article/0,,9065-2473189,00.html'
    link9_result = %{<a href="#{link9_raw}">#{CGI.escapeHTML(link9_raw)}</a>}
    link10_raw    = 'http://www.mail-archive.com/ruby-talk@ruby-lang.org/'
    link10_result = %{<a href="#{link10_raw}">#{CGI.escapeHTML(link10_raw)}</a>}

    assert_linked %(Go to #{link_result} and say hello to #{email_result}), "Go to #{link_raw} and say hello to #{email_raw}"
    assert_linked %(Link #{link_result}), "Link #{link_raw}"
    assert_linked %(#{link_result} Link), "#{link_raw} Link"
    assert_linked %(Go to #{link_result}.), %(Go to #{link_raw}.)
    assert_linked %(Go to #{link_result}, then say hello to #{email_result}.), %(Go to #{link_raw}, then say hello to #{email_raw}.)
    assert_linked %(Link #{link2_result}), "Link #{link2_raw}"
    assert_linked %(#{link2_result} Link), "#{link2_raw} Link"
    assert_linked %(Go to #{link2_result}.), %(Go to #{link2_raw}.)
    assert_linked %(Say hello to #{email_result}, then go to #{link2_result}.), %(Say hello to #{email_raw}, then go to #{link2_raw}.)
    assert_linked %(Link #{link3_result}), "Link #{link3_raw}"
    assert_linked %(#{link3_result} Link), "#{link3_raw} Link"
    assert_linked %(Go to #{link3_result}.), %(Go to #{link3_raw}.)
    assert_linked %(Go to #{link3_result}. seriously, #{link3_result}? i think I'll say hello to #{email_result}. instead.), %(Go to #{link3_raw}. seriously, #{link3_raw}? i think I'll say hello to #{email_raw}. instead.)
    assert_linked %(Link #{link4_result}), "Link #{link4_raw}"
    assert_linked %(#{link4_result} Link), "#{link4_raw} Link"
    assert_linked %(#{link5_result} Link), "#{link5_raw} Link"
    assert_linked %(#{link6_result} Link), "#{link6_raw} Link"
    assert_linked %(#{link7_result} Link), "#{link7_raw} Link"
    assert_linked %(Link #{link8_result}), "Link #{link8_raw}"
    assert_linked %(#{link8_result} Link), "#{link8_raw} Link"
    assert_linked %(Go to #{link8_result}.), %(Go to #{link8_raw}.)
    assert_linked %(Go to #{link8_result}. seriously, #{link8_result}? i think I'll say hello to #{email_result}. instead.), %(Go to #{link8_raw}. seriously, #{link8_raw}? i think I'll say hello to #{email_raw}. instead.)
    assert_linked %(Link #{link9_result}), "Link #{link9_raw}"
    #assert_linked %(#{link9_result} Link), "#{link9_raw} Link"
    assert_linked %(Go to #{link9_result}.), %(Go to #{link9_raw}.)
    #assert_linked %(Go to #{link9_result}. seriously, #{link9_result}? i think I'll say hello to #{email_result}. instead.), %(Go to #{link9_raw}. seriously, #{link9_raw}? i think I'll say hello to #{email_raw}. instead.)
    #assert_linked %(#{link10_result} Link), "#{link10_raw} Link"
    assert_linked email2_result, email2_raw
    assert_linked "#{link_result} #{link_result} #{link_result}", "#{link_raw} #{link_raw} #{link_raw}"
    assert_linked '<a href="http://www.rubyonrails.com">Ruby On Rails</a>', '<a href="http://www.rubyonrails.com">Ruby On Rails</a>'
  end
end
