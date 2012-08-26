# encoding: UTF-8
require "Php"

describe 'Php' do
  it 'return version' do
      Php::VERSION.should == [0,0,1]
  end

  describe 'eval' do 
      it 'return any types' do
          Php.eval('true').should be_true 
          Php.eval('false').should be_false 
          Php.eval('null').should be_nil 
          Php.eval('1').should == 1 
      end
     
      it 'raise error with invalid php code' do
          proc {
              Php.eval("i n v a l i d")
          }.should raise_error
      end 
  end 

  describe 'call' do
      it 'return phpversion' do
          Php.call("phpversion").should == "5.4.5"
      end 

      it 'bin2hex return hex string' do
          Php.call("bin2hex", "+").should == "2b"
      end 
    
    
      it 'intval return integer value' do
          Php.call("intval", "123a").should == 123
          Php.call("intval", 123).should == 123
      end 
      
      it 'floatval return float value' do
          Php.call("floatval", "-8.93").should == -8.93
          Php.call("floatval", -8.93).should == -8.93
      end 
    
      it 'call with integer' do
          Php.call("pow", 2, 8).should == 256
      end 
    
      it 'call with array' do
          Php.call("array_diff", [1,2,3,4,5], [3,4]).should == [1,2,5]
      end 

      it 'raise error with invalid php code' do
          proc {
              Php.call("i n v a l i d")
          }.should raise_error
      end 
  end

  describe 'output' do
    it 'handling output' do
        capture = nil
        Php.setOutputHandler(Proc.new { |output|
          capture = output
        })
        
        Php.eval('print("hoge")')
        capture.should == 'hoge'
    end 
  end

  describe 'error' do
    it 'handling error' do
        capture = []
        Php.setErrorHandler(Proc.new { |error|
          capture << error
        })
        
        Php.eval('trigger_error("hoge")')
        capture.should == [
          "PHP Notice:  hoge in  on line 1",
          "PHP Stack trace:",
          "PHP   1. {main}() :0",
          "PHP   2. trigger_error() :1"
        ]
    end 
  end

end 



