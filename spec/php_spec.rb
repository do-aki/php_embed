# encoding: UTF-8
require 'php_embed'

describe PhpEmbed do

  describe 'eval' do 
      it 'raise error with invalid PhpEmbed code' do
          proc {
              PhpEmbed.eval("i n v a l i d")
          }.should raise_error
      end 
  end 

  describe 'eval and call' do
      it '1' do 
          PhpEmbed.eval('function hoge(){ return 1; }');
          PhpEmbed.call('hoge').should == 1;
      end
      it 'any value' do 
          PhpEmbed.eval('function hoge2($arg){ return $arg; }');
          PhpEmbed.call('hoge2', 1).should == 1;
          PhpEmbed.call('hoge2', 'a').should == 'a';
          PhpEmbed.call('hoge2', []).to_a.should == [];
          PhpEmbed.call('hoge2', [1]).to_a.should == [1];
          PhpEmbed.call('hoge2', {0=>1,1=>2}).to_a.should == [1,2];
          PhpEmbed.call('hoge2', {10=>2}).to_h.should == {10=>2};
          PhpEmbed.call('hoge2', {'a'=>1}).to_h.should == {'a'=>1};
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
          PhpEmbed.call("array_diff", [1,2,3,4,5], [3,4]).to_h.should == {0=>1,1=>2,4=>5}
      end 

      it 'call by symbol' do
          PhpEmbed.call(:intval, 123).should == 123
      end

      it 'raise error with invalid PhpEmbed code' do
          proc {
              PhpEmbed.call("i n v a l i d")
          }.should raise_error
      end 
  end

  describe 'output' do
    it 'handling output' do
        capture = nil
        PhpEmbed.setOutputHandler(Proc.new { |output|
          capture = output
        })
        
        PhpEmbed.eval('print("hoge");')
        capture.should == 'hoge'
    end 
  end

  describe 'error' do
    it 'handling error' do
        capture = []
        PhpEmbed.setErrorHandler(Proc.new { |error|
          capture << error
        })
        
        PhpEmbed.eval('trigger_error("hoge");')
        capture.join('').should match('PHP Notice:')
    end 
  end

  


end 



