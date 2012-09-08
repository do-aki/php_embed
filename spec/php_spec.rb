# encoding: UTF-8
require 'php_embed'

describe 'PhpEmbed' do

  describe 'eval' do 
      it 'return any types' do
          PhpEmbed.eval('true').should be_true 
          PhpEmbed.eval('false').should be_false;
          PhpEmbed.eval('null').should be_nil
          PhpEmbed.eval('1').should == 1
          PhpEmbed.eval('array()').should == []
          PhpEmbed.eval('array(1)').should == [1]
      end
     
      it 'raise error with invalid PhpEmbed code' do
          proc {
              PhpEmbed.eval("i n v a l i d")
          }.should raise_error
      end 
  end 

  describe 'call' do
      it 'bin2hex return hex string' do
          PhpEmbed.call("bin2hex", "+").should == "2b"
      end 
    
      it 'intval return integer value' do
          PhpEmbed.call("intval", "123a").should == 123
          PhpEmbed.call("intval", 123).should == 123
      end 
      
      it 'floatval return float value' do
          PhpEmbed.call("floatval", "-8.93").should == -8.93
          PhpEmbed.call("floatval", -8.93).should == -8.93
      end 
    
      it 'call with integer' do
          PhpEmbed.call("pow", 2, 8).should == 256
      end 
    
      it 'call with array' do
          PhpEmbed.call("array_diff", [1,2,3,4,5], [3,4]).should == [1,2,5]
      end 

#      it 'raise error with invalid PhpEmbed code' do
#          proc {
#              PhpEmbed.call("i n v a l i d")
#          }.should raise_error
#      end 
  end

  describe 'output' do
    it 'handling output' do
        capture = nil
        PhpEmbed.setOutputHandler(Proc.new { |output|
          capture = output
        })
        
        PhpEmbed.eval('print("hoge")')
        capture.should == 'hoge'
    end 
  end

  describe 'error' do
    it 'handling error' do
        capture = []
        PhpEmbed.setErrorHandler(Proc.new { |error|
          capture << error
        })
        
        PhpEmbed.eval('trigger_error("hoge")')
        capture.join('').should match('PHP Notice:')
    end 
  end

end 



