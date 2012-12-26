require 'php_embed/php'
require 'php_embed/version'


module PhpEmbed
  def self.Value(value)
    self::Value.new(value)
  end
end
