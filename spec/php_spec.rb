# encoding: UTF-8
require "Php"

describe 'Php' do
  it 'return version' do
      Php::VERSION.should == [0,0,1]
  end

  describe 'eval' do 
      it 'return true with valid php code' do
          php = Php.new
          php.eval(";").should be_true;
      end
     
      it 'return false with invalid php code' do
          proc {
              php = Php.new
              php.eval("invalid").should be_false;
          }.should raise_error
      end 
  end 

  describe 'call' do
      it 'return phpversion' do
          php = Php.new
          php.call("phpversion").should == "5.4.5";
      end 

      it 'bin2hex return hex string' do
          php = Php.new
          php.call("bin2hex", "+").should == "2b";
      end 
    
    
      it 'intval return integer value' do
          php = Php.new
          php.call("intval", "123a").should == 123;
      end 
      
      it 'floatval return float value' do
          php = Php.new
          php.call("floatval", "-8.93").should == -8.93;
      end 
    
      it 'call with integer' do
          php = Php.new
          php.call("pow", 2, 8).should == 256;
      end 
    
      it 'call with array' do
          php = Php.new
          php.call("array_diff", [1,2,3,4,5], [3,4]).should == [1,2,5];
      end 
  end

  describe 'output' do
    it 'handling output' do
        capture = nil
        Php.setOutputHandler(Proc.new { |output|
          capture = output
        })
        
        php = Php.new
        php.eval('echo "hoge";')
        capture.should == 'hoge'
    end 
  end

  describe 'error' do
    it 'handling error' do
        capture = []
        Php.setErrorHandler(Proc.new { |error|
          capture << error
        })
        
        php = Php.new
        php.eval('trigger_error("hoge");')
        capture.should == [
          "PHP Notice:  hoge in  on line 1",
          "PHP Stack trace:",
          "PHP   1. {main}() :0",
          "PHP   2. trigger_error() :1"
        ]
    end 
  end

end 



