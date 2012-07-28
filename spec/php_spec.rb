# encoding: UTF-8
require "Php"

describe 'Php' do
  it 'return version' do
      Php::VERSION.should == [0,0,1]
  end

  it 'eval return true with valid php code' do
      php = Php.new
      php.eval(";").should be_true;
  end
 
  it 'eval return false with invalid php code' do
      proc {
          php = Php.new
          php.eval("invalid").should be_false;
      }.should raise_error
  end 

  it 'call1' do
      php = Php.new
      php.call("bin2hex", "+").should == "2b";
  end 

  it 'call2' do
      php = Php.new
      php.call("phpversion").should == "5.4.5";
  end 

  it 'call_int' do
      php = Php.new
      php.call("intval", "123a").should == 123;
  end 
  
  it 'call_float' do
      php = Php.new
      php.call("floatval", "-8.93").should == -8.93;
  end 

  it 'call_pow' do
      php = Php.new
      php.call("pow", 2, 8).should == 256;
  end 
end 



