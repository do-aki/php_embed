# encoding: UTF-8
require 'php_embed'

describe PhpEmbed do

  describe 'eval' do 
      it 'raise error with invalid PhpEmbed code' do
          proc {
              PhpEmbed.eval("i n v a l i d")
          }.should raise_error(PhpEmbed::SyntaxError)
      end 
  end 

  describe 'eval and call' do
      it 'return fixnum' do 
          PhpEmbed.eval('function ret_1(){ return 1; }');
          PhpEmbed.call('ret_1').should == 1;
      end
      it 'return string' do 
          PhpEmbed.eval('function ret_string(){ return "string"; }');
          PhpEmbed.call('ret_string').should == 'string';
      end
      it 'return array' do 
          PhpEmbed.eval('function ret_array(){ return array(1,2); }');
          PhpEmbed.call('ret_array').to_a.should == [1,2];
      end
      it 'return hash' do 
          PhpEmbed.eval('function ret_hash(){ return array("a"=>1,"b"=>2); }');
          PhpEmbed.call('ret_hash').to_a.should == [1, 2];
          PhpEmbed.call('ret_hash').to_h.should == {'a'=>1,'b'=>2};
      end
      it 'return arg' do 
          PhpEmbed.eval('function ret_arg($arg){ return $arg; }');
          PhpEmbed.call('ret_arg', 1).should == 1;
          PhpEmbed.call('ret_arg', 'a').should == 'a';
          PhpEmbed.call('ret_arg', []).to_a.should == [];
          PhpEmbed.call('ret_arg', [1]).to_a.should == [1];
          PhpEmbed.call('ret_arg', {0=>1,1=>2}).to_a.should == [1,2];
          PhpEmbed.call('ret_arg', {10=>2}).to_h.should == {10=>2};
          PhpEmbed.call('ret_arg', {'a'=>1}).to_h.should == {'a'=>1};

          PhpEmbed.call('ret_arg', PhpEmbed::Value('a')).should == 'a';
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
          }.should raise_error(PhpEmbed::MissingError)
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

  describe 'require' do
    it 'require' do
      PhpEmbed.require(File.dirname(File.expand_path(__FILE__)) + '/require.php')
      PhpEmbed.call(:required_function).should == 'ok'
    end
    it 'invalid argument' do
      proc {
        PhpEmbed.require(nil)
      }.should raise_error(ArgumentError)
    end
  end

end 

