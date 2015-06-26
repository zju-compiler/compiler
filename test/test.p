program hello;
var a : integer;

function A2(b1 : integer) : integer;
var 
	b : integer;

function B2(b2 : integer) : integer;

begin
	a := 123;
	b := 321;
end
;

begin
	B2(3);
	writeln(b);
end
;

function A1(a1 : integer) : integer;

	function B1(b2 : integer) : integer;

	begin
		A2(3);
	end
	;

	begin
		B1(3);
	end
	;
begin
	A1(4);
	writeln(a);
end.



