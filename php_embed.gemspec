# -*- encoding: utf-8 -*-
require File.expand_path('../lib/php_embed/version', __FILE__)

Gem::Specification.new do |gem|
  gem.authors       = ["do_aki"]
  gem.email         = ["do.hiroaki@gmail.com"]
  gem.description   = %q{execute PHP code in Ruby code}
  gem.summary       = %q{execute PHP code in Ruby code}
  gem.homepage      = %q{https://github.com/do-aki/php_embed}

  gem.extensions    = ['ext/php_embed/extconf.rb']
  gem.files         = `git ls-files`.split($\)
  gem.executables   = gem.files.grep(%r{^bin/}).map{ |f| File.basename(f) }
  gem.test_files    = gem.files.grep(%r{^(test|spec|features)/})
  gem.name          = "php_embed"
  gem.require_paths = ['lib']
  gem.version       = PhpEmbed::VERSION

end
