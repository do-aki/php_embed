describe PhpEmbed::Value do
  it 'convert nil' do
    PhpEmbed::Value.new(nil).to_s.should == 'null';
  end
  it 'convert boolean' do
    PhpEmbed::Value.new(false).to_s.should == 'false';
    PhpEmbed::Value.new(true).to_s.should == 'true';
  end

  it 'convert scalar' do
    PhpEmbed::Value.new(1).to_s.should == '1';
    PhpEmbed::Value.new("a").to_s.should == "'a'";
    PhpEmbed::Value.new(:a).to_s.should == "'a'";
  end

  it 'convert array' do
    PhpEmbed::Value.new([]).to_s.should == 'array()';
    PhpEmbed::Value.new([1,2,3]).to_s.should == 'array(1,2,3)';
    PhpEmbed::Value.new(['a', 'b']).to_s.should == "array('a','b')";
  end

  it 'convert hash' do
    PhpEmbed::Value.new({}).to_s.should == 'array()';
    PhpEmbed::Value.new({1=>'a'}).to_s.should == "array(1=>'a')";
    PhpEmbed::Value.new({'a'=>1}).to_s.should == "array('a'=>1)";
    PhpEmbed::Value.new({:a=>1}).to_s.should == "array('a'=>1)";
  end
end

