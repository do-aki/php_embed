# encoding: UTF-8
require 'php_embed'

describe PhpEmbed::Value do
  it 'convert nil' do
    PhpEmbed::Value.to_php(nil).should == 'null'
  end

  it 'convert boolean' do
    PhpEmbed::Value.to_php(false).should == 'false'
    PhpEmbed::Value.to_php(true).should == 'true'
  end

  it 'convert scalar' do
    PhpEmbed::Value.to_php(1).should == '1'
    PhpEmbed::Value.to_php("a").should == "'a'"
    PhpEmbed::Value.to_php(:a).should == "'a'"
  end

  it 'convert array' do
    PhpEmbed::Value.to_php([]).should == 'array()'
    PhpEmbed::Value.to_php([1,2,3]).should == 'array(1,2,3)'
    PhpEmbed::Value.to_php(['a', 'b']).should == "array('a','b')"
  end

  it 'convert hash' do
    PhpEmbed::Value.to_php({}).should == 'array()'
    PhpEmbed::Value.to_php({1=>'a'}).should == "array(1=>'a')"
    PhpEmbed::Value.to_php({'a'=>1}).should == "array('a'=>1)"
    PhpEmbed::Value.to_php({:a=>1}).should == "array('a'=>1)"
  end


  it 'to_s' do
    PhpEmbed::Value.new('test').to_s.should == 'test'
    PhpEmbed::Value.new(:test).to_s.should == 'test'

    PhpEmbed::Value.new(123).to_s.should == '123'
    PhpEmbed::Value.new(nil).to_s.should == ''

    PhpEmbed::Value.new(false).to_s.should == ''
    PhpEmbed::Value.new(true).to_s.should == '1'
  end

  it 'to_i' do
    PhpEmbed::Value.new('test').to_i.should == 0
    PhpEmbed::Value.new('10test').to_i.should == 10

    PhpEmbed::Value.new(123).to_i.should == 123
    PhpEmbed::Value.new(nil).to_i.should == 0
    PhpEmbed::Value.new(true).to_i.should == 1
    PhpEmbed::Value.new(false).to_i.should == 0
    PhpEmbed::Value.new(12.3).to_i.should == 12
  end

  it 'to_b' do
    PhpEmbed::Value.new(0).to_b.should be false
    PhpEmbed::Value.new([]).to_b.should be false
    PhpEmbed::Value.new({}).to_b.should be false
    PhpEmbed::Value.new(true).to_b.should be true
    PhpEmbed::Value.new(false).to_b.should be false
    PhpEmbed::Value.new(nil).to_b.should be false
  end

  it 'to_a' do
    PhpEmbed::Value.new([]).to_a.should == []
    PhpEmbed::Value.new([1]).to_a.should == [PhpEmbed::Value.new(1)]
    PhpEmbed::Value.new([1,"2",nil]).to_a
      .should == [PhpEmbed::Value.new(1), PhpEmbed::Value.new("2"), PhpEmbed::Value(nil)]
  end

  it 'to_h' do
    PhpEmbed::Value.new([]).to_h.should == {}
    PhpEmbed::Value.new([1,"2",nil]).to_h
      .should == {0=>PhpEmbed::Value.new(1), 1=>PhpEmbed::Value.new("2"), 2=>PhpEmbed::Value(nil)}
    PhpEmbed::Value.new({a:1}).to_h
      .should == {'a'=>PhpEmbed::Value.new(1)}
  end

  it 'to_f' do
    PhpEmbed::Value.new('test').to_f.should == 0
    PhpEmbed::Value.new(12.3).to_f.should == 12.3
  end

  it 'equqal' do
    PhpEmbed::Value.new(true).should == PhpEmbed::Value.new(true)
    PhpEmbed::Value.new(false).should == PhpEmbed::Value.new(false)

    PhpEmbed::Value.new(nil).should == PhpEmbed::Value.new(false)

    PhpEmbed::Value.new(nil).should == nil
    PhpEmbed::Value.new(true).should == true
    PhpEmbed::Value.new(false).should == false
    PhpEmbed::Value.new(1).should == 1
    PhpEmbed::Value.new('a').should == 'a'
  end
end

