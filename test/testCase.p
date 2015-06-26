////////////////////////////////////////////////////
test function
////////////////////////////////////////////////////
program HelloWorld;
type 
	int = integer;
function max(num1, num2 :integer; var xx:char) :integer;
var
	result:int;
function min(num1, num2:integer):integer;
var 
	r : int;
	begin
		min := num2;
	end;
begin
	max := num1;
end;
begin	

end.


program HelloWorld;
type 
	int = integer;
var 
	n1 : int;
	n2 : int;
	n3 : int;
procedure max(num1, num2 :integer);
var
	result:int;
begin
	result := num1;
end;
begin	
	max(n1,n2);
end.


program HelloWorld;
type 
	int = integer;
var 
	n1 : int;
	n2 : int;
	n3 : int;
procedure max(num1, num2 :integer; var n :integer);
var
	result:int;
begin
	n := 1;
end;
begin	
	n1 := -1;
	n2 := -n1+n2;
	max(n1,n2,n3);
end.

////////////////////////////////////////////////////
test binary expr
////////////////////////////////////////////////////


////////////////////////////////////////////////////
test unary expr
////////////////////////////////////////////////////



////////////////////////////////////////////////////
test if stmt
////////////////////////////////////////////////////

program HelloWorld;
type 
	int = integer;
var 
	result:int;

procedure min(num1, num2:integer;var r:integer);
begin
	if (num1 < num2) then r := num1 else r:= num2;
end;

begin	
	min(1,2);
	writeln(result);
end.


////////////////////////////////////////////////////
test while stmt
////////////////////////////////////////////////////
program HelloWorld;
type 
	int = integer;
var 
	s : int;
	result:int;
begin	
	s := 0;
	result := 0;
	while s < 100 do
	begin 
		result := result + s;
		s := s + 1;
	end;
	writeln(result);
end.


////////////////////////////////////////////////////
test for stmt
////////////////////////////////////////////////////
program HelloWorld;
type 
	int = integer;
var 
	s : int;
	result:int;
begin	
	s := 0;
	result := 0;
	for s:=0 to 100 do
	begin
		result := result + s;
	end;
	writeln(result);
end.


////////////////////////////////////////////////////
test repeat stmt
////////////////////////////////////////////////////

program HelloWorld;
type 
	int = integer;
var 
	s : int;
	result:int;
begin	
	s := 0;
	result := 0;
	repeat
		result := result + s;
		s := s + 1;
	until s >= 100;
	writeln(result);
end.
////////////////////////////////////////////////////
test copyblock
////////////////////////////////////////////////////



program HelloWorld;
type 
	book = record 
		id : array [1..20] of char;
	end;
var 
	b : book;
	ar : array[char] of book;
	s : string;
begin	
	ar[0] := s;
end.





program HelloWorld;
const 
	ccc = 1;
	sub = 'a';
type 
	int = integer;
	book = record 
		id : integer;
		name : char;
	end;

var 
	n1 : int;
	n2 : int;
	n3 : int;
	b : book;
	ar : array[char] of book;
procedure max(num1, num2 :integer; var n :integer);
var
	result:int;
begin
	n := 1;
end;
begin	
	n1 := -1;
	n2 := -n1+n2;
	max(n1,n2,n3);
	b.id := n1;
	ar[0] := b;
end.


////test copy record

program HelloWorld;
const 
	ccc = 1;
	sub = 'a';
type 
	int = integer;
	book = record 
		id : integer;
		id : integer;
		id : integer;
		name : char;
	end;
var 
	b : book;
	ar : array[char] of book;
begin	
	ar[0] := b;
end.


////////////////////////////////////////////////////
test type check
////////////////////////////////////////////////////


program HelloWorld;
const 
	con = 10;
var 
	i : integer;
	j : char;
	k : array[1..20] of char;
function ff(h : real) : char;
begin
end;
begin
	con := 100;
	i := ff(i,i);
	cc;
	k[i] := k;
	i := j;
end.






program HelloWorld;
type 
	int = integer;
	book = record 
		id1 : integer;
		id2 : integer;
		id3 : integer;
		name : integer;
	end;
var 
	b1,b2 : book;
	re : real;
	ar : array[1..10] of book;
	i : integer;
begin	
	b1.id1 := 1;
	ar[1] := b1;
	re := 1.1;

	re := 2.2 * re;
	writeln(b1.id1);
	writeln(re);
end.

program HelloWorld;
type 
	int = integer;
var 
	outReal : real;
	result : int;
function max(num1, num2 :integer) :integer;

function min(num1, num2:integer):integer;
var 
	r : int;
	begin
		outReal := 10.0;
		if (num1 < num2) then min := num1
		else min := num2;
	end;
begin
	result := min(2,1);
	
end;
begin	
	max(1,2);
	writeln(outReal);
	writeln(result);
end.


program HelloWorld;
type 
	int = integer;
var 
	a : array[1..20] of char;
	c : char;
begin	
	c := 'a';
	writeln(c);
end.