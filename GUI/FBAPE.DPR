program Fbape;

uses
  Forms,
  Mainfrm in 'MAINFRM.PAS' {mainform};

{$R *.RES}

begin
  Application.Title := 'X-fbape Config Tool';
  Application.CreateForm(Tmainform, mainform);
  Application.Run;
end.
