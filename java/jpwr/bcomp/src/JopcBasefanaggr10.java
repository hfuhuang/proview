package jpwr.bcomp;
import jpwr.rt.*;
import jpwr.jop.*;
import java.awt.*;
import java.awt.geom.*;
import java.awt.image.*;
import java.awt.font.*;
import javax.swing.*;
import java.awt.event.*;

public class JopcBasefanaggr10 extends JopFrame implements JopUtilityIfc {
  JPanel contentPane;
  BorderLayout borderLayout1 = new BorderLayout();
  LocalPanel localPanel = new LocalPanel();
  boolean scrollbar = false;
  Dimension size;
  pwr_indsquare	pwr_indsquare6;
  pwr_valuelong	pwr_valuelong8;
  pwr_indsquare	pwr_indsquare9;
  JopSmallbutton	jopSmallbutton11;
  pwr_indround	pwr_indround12;
  pwr_indround	pwr_indround13;
  JopSmallbutton	jopSmallbutton14;
  pwr_mbopenobject	pwr_mbopenobject16;
  pwr_mbtrend	pwr_mbtrend17;
  pwr_mbfast	pwr_mbfast18;
  pwr_mbphoto	pwr_mbphoto19;
  pwr_mbdatasheet	pwr_mbdatasheet20;
  pwr_mbopenplc	pwr_mbopenplc21;
  pwr_mbcircuitdiagram	pwr_mbcircuitdiagram22;
  pwr_mbrtnavigator	pwr_mbrtnavigator23;
  pwr_mbhelpclass	pwr_mbhelpclass24;
  pwr_mbblockevents	pwr_mbblockevents25;
  pwr_mbhistevent	pwr_mbhistevent26;
  pwr_mbnote	pwr_mbnote27;
  pwr_mbhelp	pwr_mbhelp28;
  pwr_indround	pwr_indround29;
  pwr_valuelong	pwr_valuelong30;
  pwr_rod	pwr_rod31;
  JopSmallbutton	jopSmallbutton32;
  pwr_valuelong	pwr_valuelong33;
  pwr_indsquare	pwr_indsquare34;
  pwr_indround	pwr_indround36;
  pwr_contactor2	pwr_contactor237;
  pwr_safetyswitch	pwr_safetyswitch38;
  pwr_fananim	pwr_fananim39;
  pwr_fananim2	pwr_fananim240;
  pwr_indsquare	pwr_indsquare42;
  pwr_menubar	pwr_menubar43;
  pwr_pulldownmenu	pwr_pulldownmenu44;
  pwr_pulldownmenu	pwr_pulldownmenu45;
  pwr_pulldownmenu	pwr_pulldownmenu46;
  pwr_pulldownmenu	pwr_pulldownmenu47;
  pwr_pulldownmenu	pwr_pulldownmenu48;
  pwr_motor2	pwr_motor249;
  Grp166_	grp166_50;
  pwr_valuesmall	pwr_valuesmall51;
  pwr_valuesmall	pwr_valuesmall52;
  JopSmallbutton	jopSmallbutton56;
  pwr_mbsimulate	pwr_mbsimulate57;
  pwr_indsquare	pwr_indsquare58;
  Grp173_	grp173_59;
  pwr_fuse3	pwr_fuse360;
  Grp182_	grp182_61;
  JopSmallbutton	jopSmallbutton62;
  public JopcBasefanaggr10( JopSession session, String instance, boolean scrollbar) {
    super( session, instance);
    this.scrollbar = scrollbar;
    geInit();
  }
  public JopcBasefanaggr10( JopSession session, String instance, boolean scrollbar, boolean noinit) {
    super( session, instance);
    this.scrollbar = scrollbar;
    if ( !noinit)
      geInit();
  }
  public void geInit() {
    JopSpider.setSystemName( "");
    engine.setAnimationScanTime( 200);
    engine.setScanTime( 500);
    size = new Dimension( 417, 592);
    Dimension dsize = new Dimension(localPanel.original_width,localPanel.original_height);
    this.addComponentListener(new AspectRatioListener(this,size));
    contentPane = (JPanel) this.getContentPane();
    contentPane.setLayout(borderLayout1);
    if ( scrollbar)
      contentPane.add( new JScrollPane(localPanel), BorderLayout.CENTER);
    else
      contentPane.add(localPanel, BorderLayout.CENTER);
    contentPane.setOpaque(true);
    localPanel.setLayout( new RatioLayout()); // scaletest
    localPanel.setOpaque(true);
    localPanel.setBackground(GeColor.getColor(33, GeColor.NO_COLOR));
    this.setSize(size);
    if ( engine.isInstance())
      setTitle( engine.getInstance());
    else
      this.setTitle("JopcBasefanaggr10");
    pwr_indsquare6 = new pwr_indsquare(session);
    pwr_indsquare6.setBounds(new Rectangle(185,317,16,16));
    pwr_indsquare6.setShadow(1);
    localPanel.add(pwr_indsquare6, new Proportion(pwr_indsquare6.getBounds(), dsize));
    pwr_valuelong8 = new pwr_valuelong(session);
    pwr_valuelong8.setBounds(new Rectangle(16,46,369,22));
    pwr_valuelong8.setFillColor(33);
    pwr_valuelong8.setBorderColor(36);
    localPanel.add(pwr_valuelong8, new Proportion(pwr_valuelong8.getBounds(), dsize));
    pwr_indsquare9 = new pwr_indsquare(session);
    pwr_indsquare9.setBounds(new Rectangle(185,364,16,16));
    pwr_indsquare9.setFillColor(294);
    pwr_indsquare9.setShadow(1);
    localPanel.add(pwr_indsquare9, new Proportion(pwr_indsquare9.getBounds(), dsize));
    jopSmallbutton11 = new JopSmallbutton(session);
    jopSmallbutton11.setBounds(new Rectangle(28,167,48,21));
    jopSmallbutton11.setFillColor(32);
    localPanel.add(jopSmallbutton11, new Proportion(jopSmallbutton11.getBounds(), dsize));
    pwr_indround12 = new pwr_indround(session);
    pwr_indround12.setBounds(new Rectangle(89,171,18,17));
    pwr_indround12.setShadow(1);
    localPanel.add(pwr_indround12, new Proportion(pwr_indround12.getBounds(), dsize));
    pwr_indround13 = new pwr_indround(session);
    pwr_indround13.setBounds(new Rectangle(89,193,18,17));
    pwr_indround13.setShadow(1);
    localPanel.add(pwr_indround13, new Proportion(pwr_indround13.getBounds(), dsize));
    jopSmallbutton14 = new JopSmallbutton(session);
    jopSmallbutton14.setBounds(new Rectangle(115,191,61,21));
    jopSmallbutton14.setFillColor(32);
    localPanel.add(jopSmallbutton14, new Proportion(jopSmallbutton14.getBounds(), dsize));
    pwr_mbopenobject16 = new pwr_mbopenobject(session);
    pwr_mbopenobject16.setBounds(new Rectangle(194,24,20,20));
    pwr_mbopenobject16.setShadow(1);
    localPanel.add(pwr_mbopenobject16, new Proportion(pwr_mbopenobject16.getBounds(), dsize));
    pwr_mbtrend17 = new pwr_mbtrend(session);
    pwr_mbtrend17.setBounds(new Rectangle(55,24,20,20));
    pwr_mbtrend17.setShadow(1);
    localPanel.add(pwr_mbtrend17, new Proportion(pwr_mbtrend17.getBounds(), dsize));
    pwr_mbfast18 = new pwr_mbfast(session);
    pwr_mbfast18.setBounds(new Rectangle(75,24,20,20));
    pwr_mbfast18.setShadow(1);
    localPanel.add(pwr_mbfast18, new Proportion(pwr_mbfast18.getBounds(), dsize));
    pwr_mbphoto19 = new pwr_mbphoto(session);
    pwr_mbphoto19.setBounds(new Rectangle(95,24,20,20));
    pwr_mbphoto19.setShadow(1);
    localPanel.add(pwr_mbphoto19, new Proportion(pwr_mbphoto19.getBounds(), dsize));
    pwr_mbdatasheet20 = new pwr_mbdatasheet(session);
    pwr_mbdatasheet20.setBounds(new Rectangle(115,24,20,20));
    pwr_mbdatasheet20.setShadow(1);
    localPanel.add(pwr_mbdatasheet20, new Proportion(pwr_mbdatasheet20.getBounds(), dsize));
    pwr_mbopenplc21 = new pwr_mbopenplc(session);
    pwr_mbopenplc21.setBounds(new Rectangle(214,24,20,20));
    pwr_mbopenplc21.setShadow(1);
    localPanel.add(pwr_mbopenplc21, new Proportion(pwr_mbopenplc21.getBounds(), dsize));
    pwr_mbcircuitdiagram22 = new pwr_mbcircuitdiagram(session);
    pwr_mbcircuitdiagram22.setBounds(new Rectangle(234,24,20,20));
    pwr_mbcircuitdiagram22.setShadow(1);
    localPanel.add(pwr_mbcircuitdiagram22, new Proportion(pwr_mbcircuitdiagram22.getBounds(), dsize));
    pwr_mbrtnavigator23 = new pwr_mbrtnavigator(session);
    pwr_mbrtnavigator23.setBounds(new Rectangle(174,24,20,20));
    pwr_mbrtnavigator23.setShadow(1);
    localPanel.add(pwr_mbrtnavigator23, new Proportion(pwr_mbrtnavigator23.getBounds(), dsize));
    pwr_mbhelpclass24 = new pwr_mbhelpclass(session);
    pwr_mbhelpclass24.setBounds(new Rectangle(254,24,20,20));
    pwr_mbhelpclass24.setShadow(1);
    localPanel.add(pwr_mbhelpclass24, new Proportion(pwr_mbhelpclass24.getBounds(), dsize));
    pwr_mbblockevents25 = new pwr_mbblockevents(session);
    pwr_mbblockevents25.setBounds(new Rectangle(154,24,20,20));
    pwr_mbblockevents25.setShadow(1);
    localPanel.add(pwr_mbblockevents25, new Proportion(pwr_mbblockevents25.getBounds(), dsize));
    pwr_mbhistevent26 = new pwr_mbhistevent(session);
    pwr_mbhistevent26.setBounds(new Rectangle(135,24,20,20));
    localPanel.add(pwr_mbhistevent26, new Proportion(pwr_mbhistevent26.getBounds(), dsize));
    pwr_mbnote27 = new pwr_mbnote(session);
    pwr_mbnote27.setBounds(new Rectangle(35,24,19,20));
    localPanel.add(pwr_mbnote27, new Proportion(pwr_mbnote27.getBounds(), dsize));
    pwr_mbhelp28 = new pwr_mbhelp(session);
    pwr_mbhelp28.setBounds(new Rectangle(16,24,20,20));
    localPanel.add(pwr_mbhelp28, new Proportion(pwr_mbhelp28.getBounds(), dsize));
    pwr_indround29 = new pwr_indround(session);
    pwr_indround29.setBounds(new Rectangle(88,216,18,17));
    pwr_indround29.setFillColor(114);
    pwr_indround29.setShadow(1);
    localPanel.add(pwr_indround29, new Proportion(pwr_indround29.getBounds(), dsize));
    pwr_valuelong30 = new pwr_valuelong(session);
    pwr_valuelong30.setBounds(new Rectangle(16,68,369,22));
    pwr_valuelong30.setFillColor(33);
    pwr_valuelong30.setBorderColor(36);
    localPanel.add(pwr_valuelong30, new Proportion(pwr_valuelong30.getBounds(), dsize));
    pwr_rod31 = new pwr_rod(session);
    pwr_rod31.setBounds(new Rectangle(138,462,39,10));
    pwr_rod31.setShadow(1);
    localPanel.add(pwr_rod31, new Proportion(pwr_rod31.getBounds(), dsize));
    jopSmallbutton32 = new JopSmallbutton(session);
    jopSmallbutton32.setBounds(new Rectangle(4,526,47,21));
    jopSmallbutton32.setBorderColor(37);
    localPanel.add(jopSmallbutton32, new Proportion(jopSmallbutton32.getBounds(), dsize));
    pwr_valuelong33 = new pwr_valuelong(session);
    pwr_valuelong33.setBounds(new Rectangle(60,525,336,23));
    pwr_valuelong33.setFillColor(33);
    pwr_valuelong33.setBorderColor(33);
    localPanel.add(pwr_valuelong33, new Proportion(pwr_valuelong33.getBounds(), dsize));
    pwr_indsquare34 = new pwr_indsquare(session);
    pwr_indsquare34.setBounds(new Rectangle(348,96,16,17));
    pwr_indsquare34.setFillColor(295);
    pwr_indsquare34.setShadow(1);
    localPanel.add(pwr_indsquare34, new Proportion(pwr_indsquare34.getBounds(), dsize));
    pwr_indround36 = new pwr_indround(session);
    pwr_indround36.setBounds(new Rectangle(187,172,18,17));
    pwr_indround36.setFillColor(294);
    pwr_indround36.setShadow(1);
    localPanel.add(pwr_indround36, new Proportion(pwr_indround36.getBounds(), dsize));
    pwr_contactor237 = new pwr_contactor2(session);
    pwr_contactor237.setBounds(new Rectangle(248,308,38,38));
    pwr_contactor237.setShadow(1);
    localPanel.add(pwr_contactor237, new Proportion(pwr_contactor237.getBounds(), dsize));
    pwr_safetyswitch38 = new pwr_safetyswitch(session);
    pwr_safetyswitch38.setBounds(new Rectangle(248,352,39,41));
    pwr_safetyswitch38.setShadow(1);
    localPanel.add(pwr_safetyswitch38, new Proportion(pwr_safetyswitch38.getBounds(), dsize));
    pwr_fananim39 = new pwr_fananim(session);
    pwr_fananim39.setBounds(new Rectangle(123,420,19,91));
    pwr_fananim39.setShadow(1);
    localPanel.add(pwr_fananim39, new Proportion(pwr_fananim39.getBounds(), dsize));
    pwr_fananim240 = new pwr_fananim2(session);
    pwr_fananim240.setBounds(new Rectangle(101,94,64,44));
    pwr_fananim240.setBorderColor(29);
    pwr_fananim240.setTextColor(0);
    pwr_fananim240.setShadow(1);
    localPanel.add(pwr_fananim240, new Proportion(pwr_fananim240.getBounds(), dsize));
    pwr_indsquare42 = new pwr_indsquare(session);
    pwr_indsquare42.setBounds(new Rectangle(348,117,16,17));
    pwr_indsquare42.setFillColor(175);
    pwr_indsquare42.setShadow(1);
    localPanel.add(pwr_indsquare42, new Proportion(pwr_indsquare42.getBounds(), dsize));
    pwr_menubar43 = new pwr_menubar(session);
    pwr_menubar43.setBounds(new Rectangle(0,0,409,24));
    pwr_menubar43.setShadow(1);
    localPanel.add(pwr_menubar43, new Proportion(pwr_menubar43.getBounds(), dsize));
    pwr_pulldownmenu44 = new pwr_pulldownmenu(session);
    pwr_pulldownmenu44.setBounds(new Rectangle(8,0,44,24));
    pwr_pulldownmenu44.setShadow(1);
    localPanel.add(pwr_pulldownmenu44, new Proportion(pwr_pulldownmenu44.getBounds(), dsize));
    pwr_pulldownmenu45 = new pwr_pulldownmenu(session);
    pwr_pulldownmenu45.setBounds(new Rectangle(328,0,69,24));
    pwr_pulldownmenu45.setShadow(1);
    localPanel.add(pwr_pulldownmenu45, new Proportion(pwr_pulldownmenu45.getBounds(), dsize));
    pwr_pulldownmenu46 = new pwr_pulldownmenu(session);
    pwr_pulldownmenu46.setBounds(new Rectangle(220,0,69,24));
    pwr_pulldownmenu46.setShadow(1);
    localPanel.add(pwr_pulldownmenu46, new Proportion(pwr_pulldownmenu46.getBounds(), dsize));
    pwr_pulldownmenu47 = new pwr_pulldownmenu(session);
    pwr_pulldownmenu47.setBounds(new Rectangle(52,0,69,24));
    pwr_pulldownmenu47.setShadow(1);
    localPanel.add(pwr_pulldownmenu47, new Proportion(pwr_pulldownmenu47.getBounds(), dsize));
    pwr_pulldownmenu48 = new pwr_pulldownmenu(session);
    pwr_pulldownmenu48.setBounds(new Rectangle(124,0,88,24));
    pwr_pulldownmenu48.setShadow(1);
    localPanel.add(pwr_pulldownmenu48, new Proportion(pwr_pulldownmenu48.getBounds(), dsize));
    pwr_motor249 = new pwr_motor2(session);
    pwr_motor249.setBounds(new Rectangle(173,430,98,74));
    localPanel.add(pwr_motor249, new Proportion(pwr_motor249.getBounds(), dsize));
    grp166_50 = new Grp166_(session);
    grp166_50.setBounds(new Rectangle(30,216,35,18));
    localPanel.add(grp166_50, new Proportion(grp166_50.getBounds(), dsize));
    pwr_valuesmall51 = new pwr_valuesmall(session);
    pwr_valuesmall51.setBounds(new Rectangle(282,196,71,17));
    localPanel.add(pwr_valuesmall51, new Proportion(pwr_valuesmall51.getBounds(), dsize));
    pwr_valuesmall52 = new pwr_valuesmall(session);
    pwr_valuesmall52.setBounds(new Rectangle(282,216,71,17));
    localPanel.add(pwr_valuesmall52, new Proportion(pwr_valuesmall52.getBounds(), dsize));
    jopSmallbutton56 = new JopSmallbutton(session);
    jopSmallbutton56.setBounds(new Rectangle(115,169,61,21));
    jopSmallbutton56.setFillColor(32);
    localPanel.add(jopSmallbutton56, new Proportion(jopSmallbutton56.getBounds(), dsize));
    pwr_mbsimulate57 = new pwr_mbsimulate(session);
    pwr_mbsimulate57.setBounds(new Rectangle(363,24,19,20));
    localPanel.add(pwr_mbsimulate57, new Proportion(pwr_mbsimulate57.getBounds(), dsize));
    pwr_indsquare58 = new pwr_indsquare(session);
    pwr_indsquare58.setBounds(new Rectangle(185,390,16,16));
    pwr_indsquare58.setFillColor(294);
    pwr_indsquare58.setShadow(1);
    localPanel.add(pwr_indsquare58, new Proportion(pwr_indsquare58.getBounds(), dsize));
    grp173_59 = new Grp173_(session);
    grp173_59.setBounds(new Rectangle(69,390,38,18));
    localPanel.add(grp173_59, new Proportion(grp173_59.getBounds(), dsize));
    pwr_fuse360 = new pwr_fuse3(session);
    pwr_fuse360.setBounds(new Rectangle(259,265,15,37));
    pwr_fuse360.setRotate(-90);
    pwr_fuse360.setShadow(1);
    localPanel.add(pwr_fuse360, new Proportion(pwr_fuse360.getBounds(), dsize));
    grp182_61 = new Grp182_(session);
    grp182_61.setBounds(new Rectangle(30,193,43,18));
    localPanel.add(grp182_61, new Proportion(grp182_61.getBounds(), dsize));
    jopSmallbutton62 = new JopSmallbutton(session);
    jopSmallbutton62.setBounds(new Rectangle(28,191,48,21));
    jopSmallbutton62.setFillColor(32);
    localPanel.add(jopSmallbutton62, new Proportion(jopSmallbutton62.getBounds(), dsize));
    pwr_indsquare6.dd.setDynType(13);
    pwr_indsquare6.dd.setActionType(2);
    pwr_indsquare6.dd.setAccess(65532);
    pwr_indsquare6.dd.setElements(new GeDynElemIfc[] {
      new GeDynDigColor(pwr_indsquare6.dd, "$object.Contactor.IndError##Boolean",176)
      ,new GeDynDigLowColor(pwr_indsquare6.dd, "$object.Contactor.IndFeedback##Boolean",29)
      ,new GeDynPopupMenu(pwr_indsquare6.dd, "$object.Contactor.Feedback")
      });
    pwr_valuelong8.dd.setDynType(1025);
    pwr_valuelong8.dd.setActionType(0);
    pwr_valuelong8.dd.setElements(new GeDynElemIfc[] {
      new GeDynValue(pwr_valuelong8.dd, "$object.Description##String80","%s")
      });
    pwr_indsquare9.dd.setDynType(5);
    pwr_indsquare9.dd.setActionType(2);
    pwr_indsquare9.dd.setAccess(65532);
    pwr_indsquare9.dd.setElements(new GeDynElemIfc[] {
      new GeDynDigLowColor(pwr_indsquare9.dd, "$object.SafetySwitch.On.ActualValue##Boolean",114)
      ,new GeDynPopupMenu(pwr_indsquare9.dd, "$object.SafetySwitch.On")
      });
    jopSmallbutton11.dd.setDynType(129);
    jopSmallbutton11.dd.setActionType(6);
    jopSmallbutton11.dd.setAccess(65532);
    jopSmallbutton11.dd.setElements(new GeDynElemIfc[] {
      new GeDynInvisible(jopSmallbutton11.dd, "$object.Mode.ManAutoHide##Boolean",0)
      ,new GeDynInvisible(jopSmallbutton11.dd, "$object.Mode.ManModDim##Boolean",1)
      ,new GeDynPopupMenu(jopSmallbutton11.dd, "$object.Mode.OpMan")
      ,new GeDynSetDig(jopSmallbutton11.dd, "$object.Mode.OpMan##Boolean")
      });
    jopSmallbutton11.setAnnot1(" Man");
    pwr_indround12.dd.setDynType(133);
    pwr_indround12.dd.setActionType(0);
    pwr_indround12.dd.setElements(new GeDynElemIfc[] {
      new GeDynInvisible(pwr_indround12.dd, "$object.Mode.ManAutoHide##Boolean",0)
      ,new GeDynDigLowColor(pwr_indround12.dd, "!$object.Mode.AutoMode##Boolean",29)
      });
    pwr_indround13.dd.setDynType(133);
    pwr_indround13.dd.setActionType(0);
    pwr_indround13.dd.setElements(new GeDynElemIfc[] {
      new GeDynInvisible(pwr_indround13.dd, "$object.Mode.ExternOrdHide##Boolean",0)
      ,new GeDynDigLowColor(pwr_indround13.dd, "$object.Mode.AutoMode##Boolean",29)
      });
    jopSmallbutton14.dd.setDynType(129);
    jopSmallbutton14.dd.setActionType(6);
    jopSmallbutton14.dd.setAccess(65532);
    jopSmallbutton14.dd.setElements(new GeDynElemIfc[] {
      new GeDynInvisible(jopSmallbutton14.dd, "$object.Mode.ManOrdHide##Boolean",0)
      ,new GeDynInvisible(jopSmallbutton14.dd, "$object.Mode.ManOrdDim##Boolean",1)
      ,new GeDynPopupMenu(jopSmallbutton14.dd, "$object.Mode.OpManStop")
      ,new GeDynSetDig(jopSmallbutton14.dd, "$object.Mode.OpManStop##Boolean")
      });
    jopSmallbutton14.setAnnot1(" Stop");
    pwr_mbopenobject16.dd.setDynType(129);
    pwr_mbopenobject16.dd.setActionType(8256);
    pwr_mbopenobject16.dd.setAccess(65532);
    pwr_mbopenobject16.dd.setElements(new GeDynElemIfc[] {
      new GeDynInvisible(pwr_mbopenobject16.dd, "$cmd(check method/method=\"Open Object\"/object=$object)",1)
      ,new GeDynCommand(pwr_mbopenobject16.dd, "call method/method=\"Open Object\"/object=$object")
      ,new GeDynTipText(pwr_mbopenobject16.dd, "Open Object")
      });
    pwr_mbtrend17.dd.setDynType(129);
    pwr_mbtrend17.dd.setActionType(8256);
    pwr_mbtrend17.dd.setAccess(65532);
    pwr_mbtrend17.dd.setElements(new GeDynElemIfc[] {
      new GeDynInvisible(pwr_mbtrend17.dd, "$cmd(check method/method=\"Trend\"/object=$object)",1)
      ,new GeDynCommand(pwr_mbtrend17.dd, "call method/method=\"Trend\"/object=$object")
      ,new GeDynTipText(pwr_mbtrend17.dd, "Trend")
      });
    pwr_mbfast18.dd.setDynType(129);
    pwr_mbfast18.dd.setActionType(8256);
    pwr_mbfast18.dd.setAccess(65532);
    pwr_mbfast18.dd.setElements(new GeDynElemIfc[] {
      new GeDynInvisible(pwr_mbfast18.dd, "$cmd(check method/method=\"Fast\"/object=$object)",1)
      ,new GeDynCommand(pwr_mbfast18.dd, "call method/method=\"Fast\"/object=$object")
      ,new GeDynTipText(pwr_mbfast18.dd, "Fast")
      });
    pwr_mbphoto19.dd.setDynType(129);
    pwr_mbphoto19.dd.setActionType(8256);
    pwr_mbphoto19.dd.setAccess(65532);
    pwr_mbphoto19.dd.setElements(new GeDynElemIfc[] {
      new GeDynInvisible(pwr_mbphoto19.dd, "$cmd(check method/method=\"Photo\"/object=$object)",1)
      ,new GeDynCommand(pwr_mbphoto19.dd, "call method/method=\"Photo\"/object=$object")
      ,new GeDynTipText(pwr_mbphoto19.dd, "Photo")
      });
    pwr_mbdatasheet20.dd.setDynType(129);
    pwr_mbdatasheet20.dd.setActionType(8256);
    pwr_mbdatasheet20.dd.setAccess(65532);
    pwr_mbdatasheet20.dd.setElements(new GeDynElemIfc[] {
      new GeDynInvisible(pwr_mbdatasheet20.dd, "$cmd(check method/method=\"DataSheet\"/object=$object)",1)
      ,new GeDynCommand(pwr_mbdatasheet20.dd, "call method/method=\"DataSheet\"/object=$object")
      ,new GeDynTipText(pwr_mbdatasheet20.dd, "DataSheet")
      });
    pwr_mbopenplc21.dd.setDynType(129);
    pwr_mbopenplc21.dd.setActionType(8256);
    pwr_mbopenplc21.dd.setAccess(65532);
    pwr_mbopenplc21.dd.setElements(new GeDynElemIfc[] {
      new GeDynInvisible(pwr_mbopenplc21.dd, "$cmd(check method/method=\"Open Plc\"/object=$object)",1)
      ,new GeDynCommand(pwr_mbopenplc21.dd, "call method/method=\"Open Plc\"/object=$object")
      ,new GeDynTipText(pwr_mbopenplc21.dd, "Open Plc")
      });
    pwr_mbcircuitdiagram22.dd.setDynType(129);
    pwr_mbcircuitdiagram22.dd.setActionType(8256);
    pwr_mbcircuitdiagram22.dd.setAccess(65535);
    pwr_mbcircuitdiagram22.dd.setElements(new GeDynElemIfc[] {
      new GeDynInvisible(pwr_mbcircuitdiagram22.dd, "$cmd(check method/method=\"CircuitDiagram\"/object=$object)",1)
      ,new GeDynCommand(pwr_mbcircuitdiagram22.dd, "call method/method=\"CircuitDiagram\"/object=$object")
      ,new GeDynTipText(pwr_mbcircuitdiagram22.dd, "CircuitDiagram")
      });
    pwr_mbrtnavigator23.dd.setDynType(129);
    pwr_mbrtnavigator23.dd.setActionType(8256);
    pwr_mbrtnavigator23.dd.setAccess(65532);
    pwr_mbrtnavigator23.dd.setElements(new GeDynElemIfc[] {
      new GeDynInvisible(pwr_mbrtnavigator23.dd, "$cmd(check method/method=\"RtNavigator\"/object=$object)",1)
      ,new GeDynCommand(pwr_mbrtnavigator23.dd, "call method/method=\"RtNavigator\"/object=$object")
      ,new GeDynTipText(pwr_mbrtnavigator23.dd, "RtNavigator")
      });
    pwr_mbhelpclass24.dd.setDynType(129);
    pwr_mbhelpclass24.dd.setActionType(8256);
    pwr_mbhelpclass24.dd.setAccess(65532);
    pwr_mbhelpclass24.dd.setElements(new GeDynElemIfc[] {
      new GeDynInvisible(pwr_mbhelpclass24.dd, "$cmd(check method/method=\"Help Class\"/object=$object)",1)
      ,new GeDynCommand(pwr_mbhelpclass24.dd, "call method/method=\"Help Class\"/object=$object")
      ,new GeDynTipText(pwr_mbhelpclass24.dd, "Help Class")
      });
    pwr_mbblockevents25.dd.setDynType(129);
    pwr_mbblockevents25.dd.setActionType(8256);
    pwr_mbblockevents25.dd.setAccess(65532);
    pwr_mbblockevents25.dd.setElements(new GeDynElemIfc[] {
      new GeDynInvisible(pwr_mbblockevents25.dd, "$cmd(check method/method=\"Block Events...\"/object=$object)",1)
      ,new GeDynCommand(pwr_mbblockevents25.dd, "call method/method=\"Block Events...\"/object=$object")
      ,new GeDynTipText(pwr_mbblockevents25.dd, "Block Events")
      });
    pwr_mbhistevent26.dd.setDynType(129);
    pwr_mbhistevent26.dd.setActionType(8256);
    pwr_mbhistevent26.dd.setAccess(65532);
    pwr_mbhistevent26.dd.setElements(new GeDynElemIfc[] {
      new GeDynInvisible(pwr_mbhistevent26.dd, "$cmd(check method/method=\"Hist Event...\"/object=$object)",0)
      ,new GeDynCommand(pwr_mbhistevent26.dd, "call method/method=\"Hist Event...\"/object=$object")
      ,new GeDynTipText(pwr_mbhistevent26.dd, "Hist Event")
      });
    pwr_mbnote27.dd.setDynType(129);
    pwr_mbnote27.dd.setActionType(8256);
    pwr_mbnote27.dd.setAccess(65532);
    pwr_mbnote27.dd.setElements(new GeDynElemIfc[] {
      new GeDynInvisible(pwr_mbnote27.dd, "$cmd(check method/method=\"Note\"/object=$object)",0)
      ,new GeDynCommand(pwr_mbnote27.dd, "call method/method=\"Note\"/object=$object")
      ,new GeDynTipText(pwr_mbnote27.dd, "Note")
      });
    pwr_mbhelp28.dd.setDynType(128);
    pwr_mbhelp28.dd.setActionType(8256);
    pwr_mbhelp28.dd.setAccess(65532);
    pwr_mbhelp28.dd.setElements(new GeDynElemIfc[] {
      new GeDynInvisible(pwr_mbhelp28.dd, "$cmd(check method/method=\"Help\"/object=$object)",1)
      ,new GeDynCommand(pwr_mbhelp28.dd, "call method/method=\"Help\"/object=$object")
      ,new GeDynTipText(pwr_mbhelp28.dd, "Help")
      });
    pwr_indround29.dd.setDynType(133);
    pwr_indround29.dd.setActionType(0);
    pwr_indround29.dd.setElements(new GeDynElemIfc[] {
      new GeDynInvisible(pwr_indround29.dd, "$object.Mode.LocalModHide##Boolean",0)
      ,new GeDynDigLowColor(pwr_indround29.dd, "$object.Mode.LocalMode##Boolean",29)
      });
    pwr_valuelong30.dd.setDynType(1025);
    pwr_valuelong30.dd.setActionType(0);
    pwr_valuelong30.dd.setElements(new GeDynElemIfc[] {
      new GeDynValue(pwr_valuelong30.dd, "$object.Specification##String80","%s")
      });
    pwr_rod31.dd.setDynType(1);
    pwr_rod31.dd.setActionType(0);
    jopSmallbutton32.dd.setDynType(128);
    jopSmallbutton32.dd.setActionType(64);
    jopSmallbutton32.dd.setAccess(65532);
    jopSmallbutton32.dd.setElements(new GeDynElemIfc[] {
      new GeDynInvisible(jopSmallbutton32.dd, "$object.Note##String80",0)
      ,new GeDynCommand(jopSmallbutton32.dd, "call method/method=\"Note\"/object=$object")
      });
    jopSmallbutton32.setAnnot1("Note !");
    pwr_valuelong33.dd.setDynType(1025);
    pwr_valuelong33.dd.setActionType(0);
    pwr_valuelong33.dd.setElements(new GeDynElemIfc[] {
      new GeDynValue(pwr_valuelong33.dd, "$object.Note##String80","%s")
      });
    pwr_indsquare34.dd.setDynType(5);
    pwr_indsquare34.dd.setActionType(2);
    pwr_indsquare34.dd.setAccess(65532);
    pwr_indsquare34.dd.setElements(new GeDynElemIfc[] {
      new GeDynDigLowColor(pwr_indsquare34.dd, "$object.Ready##Boolean",29)
      ,new GeDynPopupMenu(pwr_indsquare34.dd, "$object.Ready")
      });
    pwr_indround36.dd.setDynType(5);
    pwr_indround36.dd.setActionType(0);
    pwr_indround36.dd.setElements(new GeDynElemIfc[] {
      new GeDynDigLowColor(pwr_indround36.dd, "$object.Mode.Feedback##Boolean",29)
      });
    pwr_contactor237.dd.setDynType(32769);
    pwr_contactor237.dd.setActionType(66);
    pwr_contactor237.dd.setAccess(65535);
    pwr_contactor237.dd.setElements(new GeDynElemIfc[] {
      new GeDynDigShift(pwr_contactor237.dd, "$object.Contactor.Feedback.ActualValue##Boolean")
      ,new GeDynPopupMenu(pwr_contactor237.dd, "$object.Contactor")
      ,new GeDynCommand(pwr_contactor237.dd, "open graph /class/instance=$object.Contactor")
      });
    pwr_safetyswitch38.dd.setDynType(32769);
    pwr_safetyswitch38.dd.setActionType(66);
    pwr_safetyswitch38.dd.setAccess(65535);
    pwr_safetyswitch38.dd.setElements(new GeDynElemIfc[] {
      new GeDynDigShift(pwr_safetyswitch38.dd, "!$object.SafetySwitch.On.ActualValue##Boolean")
      ,new GeDynPopupMenu(pwr_safetyswitch38.dd, "$object.SafetySwitch")
      ,new GeDynCommand(pwr_safetyswitch38.dd, "open graph/class/instance=$object.SafetySwitch")
      });
    pwr_fananim39.dd.setDynType(65537);
    pwr_fananim39.dd.setActionType(66);
    pwr_fananim39.dd.setAccess(65532);
    pwr_fananim39.dd.setElements(new GeDynElemIfc[] {
      new GeDynAnimation(pwr_fananim39.dd, "$object.Contactor.IndFeedback##Boolean",1)
      ,new GeDynPopupMenu(pwr_fananim39.dd, "$object.Fan")
      ,new GeDynCommand(pwr_fananim39.dd, "open graph/class/inst=$object.Fan")
      });
    pwr_fananim240.dd.setDynType(99381);
    pwr_fananim240.dd.setActionType(0);
    pwr_fananim240.dd.setElements(new GeDynElemIfc[] {
      new GeDynDigError(pwr_fananim240.dd, "$object.IndError##Boolean")
      ,new GeDynDigWarning(pwr_fananim240.dd, "$object.IndWarning##Boolean")
      ,new GeDynDigLowColor(pwr_fananim240.dd, "$object.Contactor.IndFeedback##Boolean",37)
      ,new GeDynValue(pwr_fananim240.dd, "$object.Mode.IndMode##String8","%s")
      ,new GeDynDigShift(pwr_fananim240.dd, "")
      ,new GeDynAnimation(pwr_fananim240.dd, "$object.Contactor.IndFeedback##Boolean",2)
      });
    pwr_fananim240.setAnnot1("L");
    pwr_indsquare42.dd.setDynType(5);
    pwr_indsquare42.dd.setActionType(2);
    pwr_indsquare42.dd.setAccess(65532);
    pwr_indsquare42.dd.setElements(new GeDynElemIfc[] {
      new GeDynDigLowColor(pwr_indsquare42.dd, "$object.ExtInterlock##Boolean",29)
      ,new GeDynPopupMenu(pwr_indsquare42.dd, "$object.ExtInterlock")
      });
    pwr_menubar43.dd.setDynType(1);
    pwr_menubar43.dd.setActionType(0);
    pwr_pulldownmenu44.dd.setDynType(0);
    pwr_pulldownmenu44.dd.setActionType(524288);
    pwr_pulldownmenu44.dd.setAccess(65535);
    pwr_pulldownmenu44.dd.setElements(new GeDynElemIfc[] {
      new GeDynPulldownMenu(pwr_pulldownmenu44.dd, new String[] {
        "Print","Close",null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null},
        new GeDyn[] {
            new GeDyn(pwr_pulldownmenu44,1,65,65535,new GeDynElemIfc[] {
      new GeDynCommand(pwr_pulldownmenu44.dd, "print graph/class/inst=$object")
      }),
    new GeDyn(pwr_pulldownmenu44,1,262145,65535,new GeDynElemIfc[] {
      new GeDynCloseGraph(pwr_pulldownmenu44.dd)
      }),
null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null
      })
      });
    pwr_pulldownmenu44.setAnnot1("File");
    pwr_pulldownmenu45.dd.setDynType(1);
    pwr_pulldownmenu45.dd.setActionType(524288);
    pwr_pulldownmenu45.dd.setAccess(65532);
    pwr_pulldownmenu45.dd.setElements(new GeDynElemIfc[] {
      new GeDynPulldownMenu(pwr_pulldownmenu45.dd, new String[] {
        "Help","Help Class",null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null},
        new GeDyn[] {
            new GeDyn(pwr_pulldownmenu45,1,65,65535,new GeDynElemIfc[] {
      new GeDynCommand(pwr_pulldownmenu45.dd, "call method/method=\"Help\"/object=$object")
      }),
    new GeDyn(pwr_pulldownmenu45,1,65,65532,new GeDynElemIfc[] {
      new GeDynCommand(pwr_pulldownmenu45.dd, "call method/method=\"Help Class\"/object=$object")
      }),
null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null
      })
      });
    pwr_pulldownmenu45.setAnnot1("Help");
    pwr_pulldownmenu46.dd.setDynType(1);
    pwr_pulldownmenu46.dd.setActionType(524288);
    pwr_pulldownmenu46.dd.setAccess(65535);
    pwr_pulldownmenu46.dd.setElements(new GeDynElemIfc[] {
      new GeDynPulldownMenu(pwr_pulldownmenu46.dd, new String[] {
        "Contactor.Order Do","Contactor.Feedback Di",null,"SafetySwitch.On Di","Motor.TempSwitch.Switch Di","Motor.TempSensor.Value Ai",null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null},
        new GeDyn[] {
            new GeDyn(pwr_pulldownmenu46,1,65,65535,new GeDynElemIfc[] {
      new GeDynCommand(pwr_pulldownmenu46.dd, "open graph /class /inst=$object.Contactor.Order")
      }),
    new GeDyn(pwr_pulldownmenu46,129,65,65535,new GeDynElemIfc[] {
      new GeDynInvisible(pwr_pulldownmenu46.dd, "$object.Contactor.DisableFeedback##Boolean",0)
      ,new GeDynCommand(pwr_pulldownmenu46.dd, "open graph /class /inst=$object.Contactor.Feedback")
      }),
null,    new GeDyn(pwr_pulldownmenu46,1,65,65535,new GeDynElemIfc[] {
      new GeDynCommand(pwr_pulldownmenu46.dd, "open graph /class /inst=$object.SafetySwitch.On")
      }),
    new GeDyn(pwr_pulldownmenu46,129,65,33619967,new GeDynElemIfc[] {
      new GeDynInvisible(pwr_pulldownmenu46.dd, "$object.Motor.DisableTempSwitch##Boolean",0)
      ,new GeDynCommand(pwr_pulldownmenu46.dd, "open graph /class /inst=$object.Motor.TempSwitch.Switch")
      }),
    new GeDyn(pwr_pulldownmenu46,129,65,33619967,new GeDynElemIfc[] {
      new GeDynInvisible(pwr_pulldownmenu46.dd, "$object.Motor.DisableTempSensor##Boolean",0)
      ,new GeDynCommand(pwr_pulldownmenu46.dd, "open graph /class /inst=$object.Motor.TempSensor.Value")
      }),
null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null
      })
      });
    pwr_pulldownmenu46.setAnnot1("Signals");
    pwr_pulldownmenu47.dd.setDynType(1);
    pwr_pulldownmenu47.dd.setActionType(524288);
    pwr_pulldownmenu47.dd.setAccess(65535);
    pwr_pulldownmenu47.dd.setElements(new GeDynElemIfc[] {
      new GeDynPulldownMenu(pwr_pulldownmenu47.dd, new String[] {
        "Help","Note","Trend","Fast","Photo","DataSheet","Hist Event...","Block Events...","RtNavigator","Open  Object","Open Plc","CircuitDiagram","Help Class",null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null},
        new GeDyn[] {
            new GeDyn(pwr_pulldownmenu47,129,65,65535,new GeDynElemIfc[] {
      new GeDynInvisible(pwr_pulldownmenu47.dd, "$cmd(check method/method=\"Help\"/object=$object)",0)
      ,new GeDynCommand(pwr_pulldownmenu47.dd, "call method/method=\"Help\"/object=$object")
      }),
    new GeDyn(pwr_pulldownmenu47,129,65,65532,new GeDynElemIfc[] {
      new GeDynInvisible(pwr_pulldownmenu47.dd, "$cmd(check method/method=\"Note\"/object=$object)",0)
      ,new GeDynCommand(pwr_pulldownmenu47.dd, "call method/method=\"Note\"/object=$object")
      }),
    new GeDyn(pwr_pulldownmenu47,129,65,65532,new GeDynElemIfc[] {
      new GeDynInvisible(pwr_pulldownmenu47.dd, "$cmd(check method/method=\"Trend\"/object=$object)",0)
      ,new GeDynCommand(pwr_pulldownmenu47.dd, "call method/method=\"Trend\"/object=$object")
      }),
    new GeDyn(pwr_pulldownmenu47,129,65,65532,new GeDynElemIfc[] {
      new GeDynInvisible(pwr_pulldownmenu47.dd, "$cmd(check method/method=\"Fast\"/object=$object)",0)
      ,new GeDynCommand(pwr_pulldownmenu47.dd, "call method/method=\"Fast\"/object=$object")
      }),
    new GeDyn(pwr_pulldownmenu47,129,65,65532,new GeDynElemIfc[] {
      new GeDynInvisible(pwr_pulldownmenu47.dd, "$cmd(check method/method=\"Photo\"/object=$object)",0)
      ,new GeDynCommand(pwr_pulldownmenu47.dd, "call method/method=\"Photo\"/object=$object")
      }),
    new GeDyn(pwr_pulldownmenu47,129,65,65532,new GeDynElemIfc[] {
      new GeDynInvisible(pwr_pulldownmenu47.dd, "$cmd(check method/method=\"DataSheet\"/object=$object)",0)
      ,new GeDynCommand(pwr_pulldownmenu47.dd, "call method/method=\"DataSheet\"/object=$object")
      }),
    new GeDyn(pwr_pulldownmenu47,129,65,65532,new GeDynElemIfc[] {
      new GeDynInvisible(pwr_pulldownmenu47.dd, "$cmd(check method/method=\"Hist Event...\"/object=$object)",0)
      ,new GeDynCommand(pwr_pulldownmenu47.dd, "call method/method=\"Hist Event...\"/object=$object")
      }),
    new GeDyn(pwr_pulldownmenu47,129,65,65532,new GeDynElemIfc[] {
      new GeDynInvisible(pwr_pulldownmenu47.dd, "$cmd(check method/method=\"Block Events...\"/object=$object)",0)
      ,new GeDynCommand(pwr_pulldownmenu47.dd, "call method/method=\"Block Events...\"/object=$object")
      }),
    new GeDyn(pwr_pulldownmenu47,129,65,65532,new GeDynElemIfc[] {
      new GeDynInvisible(pwr_pulldownmenu47.dd, "$cmd(check method/method=\"RtNavigator\"/object=$object)",0)
      ,new GeDynCommand(pwr_pulldownmenu47.dd, "call method/method=\"RtNavigator\"/object=$object")
      }),
    new GeDyn(pwr_pulldownmenu47,129,65,65532,new GeDynElemIfc[] {
      new GeDynInvisible(pwr_pulldownmenu47.dd, "$cmd(check method/method=\"Open Object\"/object=$object)",0)
      ,new GeDynCommand(pwr_pulldownmenu47.dd, "call method/method=\"Open Object\"/object=$object")
      }),
    new GeDyn(pwr_pulldownmenu47,129,65,65532,new GeDynElemIfc[] {
      new GeDynInvisible(pwr_pulldownmenu47.dd, "$cmd(check method/method=\"Open Plc\"/object=$object)",0)
      ,new GeDynCommand(pwr_pulldownmenu47.dd, "call method/method=\"Open Plc\"/object=$object")
      }),
    new GeDyn(pwr_pulldownmenu47,129,65,65532,new GeDynElemIfc[] {
      new GeDynInvisible(pwr_pulldownmenu47.dd, "$cmd(check method/method=\"CircuitDiagram\"/object=$object)",0)
      ,new GeDynCommand(pwr_pulldownmenu47.dd, "call method/method=\"CircuitDiagram\"/object=$object")
      }),
    new GeDyn(pwr_pulldownmenu47,129,65,65532,new GeDynElemIfc[] {
      new GeDynInvisible(pwr_pulldownmenu47.dd, "$cmd(check method/method=\"Help Class\"/object=$object)",0)
      ,new GeDynCommand(pwr_pulldownmenu47.dd, "call method/method=\"Help Class\"/object=$object")
      }),
null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null
      })
      });
    pwr_pulldownmenu47.setAnnot1("Methods");
    pwr_pulldownmenu48.dd.setDynType(1);
    pwr_pulldownmenu48.dd.setActionType(524288);
    pwr_pulldownmenu48.dd.setAccess(65535);
    pwr_pulldownmenu48.dd.setElements(new GeDynElemIfc[] {
      new GeDynPulldownMenu(pwr_pulldownmenu48.dd, new String[] {
        "Contactor",null,"SafetySwitch","Motor",null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null},
        new GeDyn[] {
            new GeDyn(pwr_pulldownmenu48,1,65,65535,new GeDynElemIfc[] {
      new GeDynCommand(pwr_pulldownmenu48.dd, "open graph /class /inst=$object.Contactor")
      }),
null,    new GeDyn(pwr_pulldownmenu48,1,65,65535,new GeDynElemIfc[] {
      new GeDynCommand(pwr_pulldownmenu48.dd, "open graph /class /inst=$object.SafetySwitch")
      }),
    new GeDyn(pwr_pulldownmenu48,1,65,65535,new GeDynElemIfc[] {
      new GeDynCommand(pwr_pulldownmenu48.dd, "open graph /class /inst=$object.Motor")
      }),
null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null,null
      })
      });
    pwr_pulldownmenu48.setAnnot1("Components");
    pwr_motor249.dd.setDynType(0);
    pwr_motor249.dd.setActionType(66);
    pwr_motor249.dd.setAccess(65532);
    pwr_motor249.dd.setElements(new GeDynElemIfc[] {
      new GeDynPopupMenu(pwr_motor249.dd, "$object.Motor")
      ,new GeDynCommand(pwr_motor249.dd, "open graph/class/instance=$object.motor")
      });
    grp166_50.dd.setDynType(129);
    grp166_50.dd.setActionType(0);
    grp166_50.dd.setElements(new GeDynElemIfc[] {
      new GeDynInvisible(grp166_50.dd, "$object.Mode.LocalModHide##Boolean",0)
      });
    pwr_valuesmall51.dd.setDynType(1025);
    pwr_valuesmall51.dd.setActionType(0);
    pwr_valuesmall51.dd.setElements(new GeDynElemIfc[] {
      new GeDynValue(pwr_valuesmall51.dd, "$object.RunTimeCounter.TotalNOfStarts##UInt32","%d")
      });
    pwr_valuesmall52.dd.setDynType(1025);
    pwr_valuesmall52.dd.setActionType(0);
    pwr_valuesmall52.dd.setElements(new GeDynElemIfc[] {
      new GeDynValue(pwr_valuesmall52.dd, "$object.RunTimeCounter.TotalRunTime##DeltaTime","%1t")
      });
    jopSmallbutton56.dd.setDynType(129);
    jopSmallbutton56.dd.setActionType(6);
    jopSmallbutton56.dd.setAccess(65532);
    jopSmallbutton56.dd.setElements(new GeDynElemIfc[] {
      new GeDynInvisible(jopSmallbutton56.dd, "$object.Mode.ManOrdHide##Boolean",0)
      ,new GeDynInvisible(jopSmallbutton56.dd, "$object.Mode.ManOrdDim##Boolean",1)
      ,new GeDynPopupMenu(jopSmallbutton56.dd, "$object.Mode.OpManStart")
      ,new GeDynSetDig(jopSmallbutton56.dd, "$object.MOde.OpManStart##Boolean")
      });
    jopSmallbutton56.setAnnot1(" Start");
    pwr_mbsimulate57.dd.setDynType(129);
    pwr_mbsimulate57.dd.setActionType(8256);
    pwr_mbsimulate57.dd.setAccess(65532);
    pwr_mbsimulate57.dd.setElements(new GeDynElemIfc[] {
      new GeDynInvisible(pwr_mbsimulate57.dd, "$cmd(check method/method=\"Simulate\"/object=$object)",0)
      ,new GeDynCommand(pwr_mbsimulate57.dd, "call method/method=\"Simulate\"/object=$object")
      ,new GeDynTipText(pwr_mbsimulate57.dd, "Open simulate graph")
      });
    pwr_indsquare58.dd.setDynType(176);
    pwr_indsquare58.dd.setActionType(2);
    pwr_indsquare58.dd.setAccess(65535);
    pwr_indsquare58.dd.setElements(new GeDynElemIfc[] {
      new GeDynInvisible(pwr_indsquare58.dd, "!$object.Motor.DisplayInd##Boolean",0)
      ,new GeDynDigError(pwr_indsquare58.dd, "$object.Motor.IndError##Boolean")
      ,new GeDynDigWarning(pwr_indsquare58.dd, "$object.Motor.IndWarning##Boolean")
      ,new GeDynPopupMenu(pwr_indsquare58.dd, "$object.Motor")
      });
    grp173_59.dd.setDynType(129);
    grp173_59.dd.setActionType(0);
    grp173_59.dd.setElements(new GeDynElemIfc[] {
      new GeDynInvisible(grp173_59.dd, "!$object.Motor.DisplayInd##Boolean",0)
      });
    pwr_fuse360.dd.setDynType(1);
    pwr_fuse360.dd.setActionType(0);
    grp182_61.dd.setDynType(129);
    grp182_61.dd.setActionType(0);
    grp182_61.dd.setElements(new GeDynElemIfc[] {
      new GeDynInvisible(grp182_61.dd, "$object.Mode.ExternOrdHide##Boolean",0)
      });
    jopSmallbutton62.dd.setDynType(129);
    jopSmallbutton62.dd.setActionType(6);
    jopSmallbutton62.dd.setAccess(65532);
    jopSmallbutton62.dd.setElements(new GeDynElemIfc[] {
      new GeDynInvisible(jopSmallbutton62.dd, "$object.Mode.ManAutoHide##Boolean",0)
      ,new GeDynInvisible(jopSmallbutton62.dd, "$object.Mode.AutoModDim##Boolean",1)
      ,new GeDynPopupMenu(jopSmallbutton62.dd, "$object.Mode.OpAuto")
      ,new GeDynSetDig(jopSmallbutton62.dd, "$object.Mode.OpAuto##Boolean")
      });
    jopSmallbutton62.setAnnot1(" Auto");
    engine.setFrameReady();
  }

class LocalPanel extends JPanel {
  public LocalPanel() {}
  int fillColor = 9999;
  int originalFillColor = 9999;
  int textColor = 9999;
  int originalTextColor = 9999;
  int borderColor = 9999;
  int colorTone = 0;
  int originalColorTone = 0;
  int colorShift = 0;
  int originalColorShift = 0;
  int colorBrightness = 0;
  int originalColorBrightness = 0;
  int colorIntensity = 0;
  int originalColorIntensity = 0;
  int colorInverse = 0;
  int originalColorInverse = 0;
  int shadow = 0;
  boolean dimmed = false;
  public void setColorTone( int colorTone) {
    this.colorTone = colorTone;
    originalColorTone = colorTone;
  }
  public int getColorTone() {
    return colorTone;
  }
  public void setColorShift( int colorShift) {
    this.colorShift = colorShift;
    originalColorShift = colorShift;
  }
  public int getColorShift() {
    return colorShift;
  }
  public void setColorBrightness( int colorBrightness) {
    this.colorBrightness = colorBrightness;
    originalColorBrightness = colorBrightness;
  }
  public int getColorBrightness() {
    return colorBrightness;
  }
  public void setColorIntensity( int colorIntensity) {
    this.colorIntensity = colorIntensity;
    originalColorIntensity = colorIntensity;
  }
  public int getColorIntensity() {
    return colorIntensity;
  }
  public void setFillColor( int fillColor) {
    this.fillColor = fillColor;
    this.originalFillColor = fillColor;
  }
  public void resetFillColor() {
    fillColor = originalFillColor;
  }
  public int getFillColor() {
    return fillColor;
  }
  public void setBorderColor( int borderColor) {
    this.borderColor = borderColor;
  }
  public int getBorderColor() {
    return borderColor;
  }
  public int original_width = 409;
  public int original_height = 548;
  double rotate;
  public void setRotate( double rotate) {
    if ( rotate < 0)
      this.rotate = rotate % 360 + 360;
    else
      this.rotate = rotate % 360;
  }
  public double getRotate() { return rotate;}
  Shape[] shapes = new Shape[] { 
    new Line2D.Float( 267F, 255F, 267F, 311F),
    new Rectangle2D.Float(30.5695F, 113.881F, 188.144F, 7.6018F),
    new Polygon( new int[] { 31, 219, 217, 32, 32, 31}, new int[] { 114, 114, 115, 115, 120, 121}, 6), 
    new Polygon( new int[] { 219, 219, 217, 217, 32, 31}, new int[] { 121, 114, 115, 120, 120, 121}, 6), 
    new Rectangle2D.Float(19.6569F, 245.175F, 360.366F, 277.42F),
    new Polygon( new int[] { 20, 380, 379, 20, 20, 20}, new int[] { 245, 245, 246, 246, 522, 523}, 6), 
    new Polygon( new int[] { 380, 380, 379, 379, 20, 20}, new int[] { 523, 245, 246, 522, 522, 523}, 6), 
    new Rectangle2D.Float(19.5789F, 144.332F, 199.614F, 96.2F),
    new Polygon( new int[] { 20, 219, 218, 21, 21, 20}, new int[] { 144, 144, 145, 145, 240, 241}, 6), 
    new Polygon( new int[] { 219, 219, 218, 218, 21, 20}, new int[] { 241, 144, 145, 240, 240, 241}, 6), 
    new Rectangle2D.Float(221.264F, 144.515F, 158.914F, 96.2434F),
    new Polygon( new int[] { 221, 380, 379, 222, 222, 221}, new int[] { 145, 145, 145, 145, 240, 241}, 6), 
    new Polygon( new int[] { 380, 380, 379, 379, 222, 221}, new int[] { 241, 145, 145, 240, 240, 241}, 6), 
    new Line2D.Float( 217.081F, 166.041F, 20.2755F, 166.041F),
    new Line2D.Float( 227F, 432F, 257F, 432F),
    new Line2D.Float( 267F, 422F, 267F, 393F),
    new Arc2D.Float(247F, 412F, 20F, 20F, 270F, 90F, Arc2D.OPEN),
    new Line2D.Float( 227F, 432F, 257F, 432F),
    new Line2D.Float( 227F, 432F, 257F, 432F),
    new Line2D.Float( 267F, 422F, 267F, 393F),
    new Line2D.Float( 267F, 422F, 267F, 393F),
    new Arc2D.Float(247F, 412F, 20F, 20F, 270F, 90F, Arc2D.OPEN),
    new Arc2D.Float(247F, 412F, 20F, 20F, 270F, 90F, Arc2D.OPEN),
    new Line2D.Float( 267F, 358F, 267F, 345F),
    new Line2D.Float( 268F, 358F, 268F, 345F),
    new Line2D.Float( 266F, 358F, 266F, 345F),
    new Line2D.Float( 267F, 358F, 267F, 345F),
    new Line2D.Float( 267F, 358F, 267F, 345F),
  };
  public void paint(Graphics g1) {
    Graphics2D g = (Graphics2D) g1;
    Component c;
    Point p;
    paintComponent(g);
    for ( int i = 0; i < getComponentCount(); i++) {
      AffineTransform save = g.getTransform();
      c = getComponent(i);
      p = c.getLocation();
      g.translate((int)p.getX(), (int)p.getY());
      c.paint(g);
      g.setTransform(save);
    }
  }
  public void paintComponent(Graphics g1) {
    Graphics2D g = (Graphics2D) g1;
    float width = getWidth();
    float height = getHeight();
    g.setRenderingHint(RenderingHints.KEY_ANTIALIASING,RenderingHints.VALUE_ANTIALIAS_ON);
    double scaleWidth =  (1.0*width/original_width);
    double scaleHeight = (1.0*height/original_height);
    AffineTransform save = g.getTransform();
    g.setColor(getBackground());
    g.fill(new Rectangle(0,0,getWidth(),getHeight()));
    g.transform( AffineTransform.getScaleInstance( scaleWidth, scaleHeight)); // scaletest
    AffineTransform save_tmp;
    g.setStroke( new BasicStroke(1F, BasicStroke.CAP_BUTT, BasicStroke.JOIN_ROUND));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[0]);
    {
    int fcolor = GeColor.getDrawtype(75, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    g.setColor(GeColor.getColor( fcolor));
    g.fill( shapes[1]);
    g.setColor(GeColor.shiftColor( fcolor, -2, colorInverse));
    g.fill( shapes[2]);
    g.setColor(GeColor.shiftColor( fcolor, 2, colorInverse));
    g.fill( shapes[3]);
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[1]);
    }
    {
    int fcolor = GeColor.getDrawtype(33, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    g.setColor(GeColor.shiftColor( fcolor, 2, colorInverse));
    g.fill( shapes[5]);
    g.setColor(GeColor.shiftColor( fcolor, -2, colorInverse));
    g.fill( shapes[6]);
    }
    {
    int fcolor = GeColor.getDrawtype(33, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    g.setColor(GeColor.shiftColor( fcolor, 2, colorInverse));
    g.fill( shapes[8]);
    g.setColor(GeColor.shiftColor( fcolor, -2, colorInverse));
    g.fill( shapes[9]);
    }
    {
    int fcolor = GeColor.getDrawtype(33, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    g.setColor(GeColor.shiftColor( fcolor, 2, colorInverse));
    g.fill( shapes[11]);
    g.setColor(GeColor.shiftColor( fcolor, -2, colorInverse));
    g.fill( shapes[12]);
    }
    g.setStroke( new BasicStroke(1F, BasicStroke.CAP_BUTT, BasicStroke.JOIN_ROUND));
    g.setColor(GeColor.getColor(35, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[13]);
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, textColor, dimmed));
    g.setFont(new Font("Helvetica", Font.BOLD, 12));
    g.drawString( "Contactor",71, 329);
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, textColor, dimmed));
    g.setFont(new Font("Helvetica", Font.BOLD, 12));
    g.drawString( "Safety switch",71, 377);
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, textColor, dimmed));
    g.setFont(new Font("Helvetica", Font.PLAIN, 12));
    g.drawString( "Mode",34, 161);
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, textColor, dimmed));
    g.setFont(new Font("Helvetica", Font.BOLD, 12));
    g.drawString( "Ready",261, 109);
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, textColor, dimmed));
    g.setFont(new Font("Helvetica", Font.BOLD, 12));
    g.drawString( "Interlock",261, 130);
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, textColor, dimmed));
    g.setFont(new Font("Helvetica", Font.BOLD, 12));
    g.drawString( "Run",235, 228);
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, textColor, dimmed));
    g.setFont(new Font("Helvetica", Font.BOLD, 12));
    g.drawString( "Starts",235, 208);
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, textColor, dimmed));
    g.setFont(new Font("Helvetica", Font.BOLD, 12));
    g.drawString( "Start",127, 183);
    g.setStroke( new BasicStroke(1F, BasicStroke.CAP_BUTT, BasicStroke.JOIN_ROUND));
    g.setColor(GeColor.getColor(2, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[14]);
    g.setStroke( new BasicStroke(1F, BasicStroke.CAP_BUTT, BasicStroke.JOIN_ROUND));
    g.setColor(GeColor.getColor(2, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[15]);
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(2, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[16]);
    g.setStroke( new BasicStroke(1F, BasicStroke.CAP_BUTT, BasicStroke.JOIN_ROUND));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[17]);
    g.setStroke( new BasicStroke(1F, BasicStroke.CAP_BUTT, BasicStroke.JOIN_ROUND));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[18]);
    g.setStroke( new BasicStroke(1F, BasicStroke.CAP_BUTT, BasicStroke.JOIN_ROUND));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[19]);
    g.setStroke( new BasicStroke(1F, BasicStroke.CAP_BUTT, BasicStroke.JOIN_ROUND));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[20]);
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[21]);
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[22]);
    g.setStroke( new BasicStroke(1F, BasicStroke.CAP_BUTT, BasicStroke.JOIN_ROUND));
    g.setColor(GeColor.getColor(2, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[23]);
    g.setStroke( new BasicStroke(1F, BasicStroke.CAP_BUTT, BasicStroke.JOIN_ROUND));
    g.setColor(GeColor.getColor(2, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[24]);
    g.setStroke( new BasicStroke(1F, BasicStroke.CAP_BUTT, BasicStroke.JOIN_ROUND));
    g.setColor(GeColor.getColor(2, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[25]);
    g.setStroke( new BasicStroke(1F, BasicStroke.CAP_BUTT, BasicStroke.JOIN_ROUND));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[26]);
    g.setStroke( new BasicStroke(1F, BasicStroke.CAP_BUTT, BasicStroke.JOIN_ROUND));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[27]);
    g.setTransform(save);
  }
  public Dimension getPreferredSize() { return size;}
  public Dimension getMinimumSize() { return size;}
}
protected class pwr_fuse3 extends GeComponent {
  // Dimension size;
  public pwr_fuse3( JopSession session)
  {
    super( session);
    size = new Dimension( 40, 16);
  }
  public int original_width = 40;
  public int original_height = 16;
  Shape[] shapes = new Shape[] { 
    new Rectangle2D.Float(2F, 2F, 36F, 12F),
    new Polygon( new int[] { 2, 38, 37, 3, 3, 2}, new int[] { 2, 2, 3, 3, 13, 14}, 6), 
    new Polygon( new int[] { 38, 38, 37, 37, 3, 2}, new int[] { 14, 2, 3, 13, 13, 14}, 6), 
    new Rectangle2D.Float(2F, 2F, 6F, 12F),
    new Polygon( new int[] { 2, 8, 8, 2, 2, 2}, new int[] { 2, 2, 2, 2, 14, 14}, 6), 
    new Polygon( new int[] { 8, 8, 8, 8, 2, 2}, new int[] { 14, 2, 2, 14, 14, 14}, 6), 
    new Rectangle2D.Float(2F, 10F, 6F, 4F),
    new Polygon( new int[] { 2, 8, 8, 2, 2, 2}, new int[] { 10, 10, 10, 10, 14, 14}, 6), 
    new Polygon( new int[] { 8, 8, 8, 8, 2, 2}, new int[] { 14, 10, 10, 14, 14, 14}, 6), 
    new Rectangle2D.Float(2F, 2F, 6F, 4F),
    new Polygon( new int[] { 2, 8, 8, 2, 2, 2}, new int[] { 2, 2, 2, 2, 6, 6}, 6), 
    new Polygon( new int[] { 8, 8, 8, 8, 2, 2}, new int[] { 6, 2, 2, 6, 6, 6}, 6), 
    new Line2D.Float( 32F, 8F, 8F, 8F),
    new Rectangle2D.Float(32F, 2F, 6F, 12F),
    new Polygon( new int[] { 32, 38, 38, 32, 32, 32}, new int[] { 2, 2, 2, 2, 14, 14}, 6), 
    new Polygon( new int[] { 38, 38, 38, 38, 32, 32}, new int[] { 14, 2, 2, 14, 14, 14}, 6), 
    new Rectangle2D.Float(32F, 10F, 6F, 4F),
    new Polygon( new int[] { 32, 38, 38, 32, 32, 32}, new int[] { 10, 10, 10, 10, 14, 14}, 6), 
    new Polygon( new int[] { 38, 38, 38, 38, 32, 32}, new int[] { 14, 10, 10, 14, 14, 14}, 6), 
    new Rectangle2D.Float(32F, 2F, 6F, 4F),
    new Polygon( new int[] { 32, 38, 38, 32, 32, 32}, new int[] { 2, 2, 2, 2, 6, 6}, 6), 
    new Polygon( new int[] { 38, 38, 38, 38, 32, 32}, new int[] { 6, 2, 2, 6, 6, 6}, 6), 
    new Line2D.Float( 38F, 6F, 2F, 6F),
    new Rectangle2D.Float(2F, 2F, 6F, 12F),
    new Polygon( new int[] { 2, 8, 8, 2, 2, 2}, new int[] { 2, 2, 2, 2, 14, 14}, 6), 
    new Polygon( new int[] { 8, 8, 8, 8, 2, 2}, new int[] { 14, 2, 2, 14, 14, 14}, 6), 
    new Rectangle2D.Float(32F, 2F, 6F, 12F),
    new Polygon( new int[] { 32, 38, 38, 32, 32, 32}, new int[] { 2, 2, 2, 2, 14, 14}, 6), 
    new Polygon( new int[] { 38, 38, 38, 38, 32, 32}, new int[] { 14, 2, 2, 14, 14, 14}, 6), 
  };
  public void paintComponent(Graphics g1) {
    animationCount = 1;
    if ( !visible)
      return;
    Graphics2D g = (Graphics2D) g1;
    float width = getWidth();
    float height = getHeight();
    g.setRenderingHint(RenderingHints.KEY_ANTIALIASING,RenderingHints.VALUE_ANTIALIAS_ON);
    if ( 45.0 <= rotate && rotate < 135.0) {
      g.translate( width, 0.0); 
      g.rotate( Math.PI * rotate/180, 0.0, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else if ( 135.0 <= rotate && rotate < 225.0)
    {
      g.rotate( Math.PI * rotate/180, width/2, height/2);
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    }
    else if ( 225.0 <= rotate && rotate < 315.0)
    {
      g.translate( -height, 0.0);
      g.rotate( Math.PI * rotate/180, height, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    if ( (dd.dynType & GeDyn.mDynType_Rotate) != 0 && dd.rotate != 0) {
      g.rotate( Math.PI * dd.rotate/180, 

           (dd.x0 - getX())*original_width/width,
           (dd.y0 - getY()) * original_height / height);
    }
    AffineTransform save = g.getTransform();
    AffineTransform save_tmp;
    int rounds = 1;
    if ( fillLevel != 1F)
      rounds = 2;
    int oldColor = 0;
    for ( int i = 0; i < rounds; i++) {
      if ( rounds == 2) {
        switch ( i) {
        case 0:
	  if ( levelColorTone != GeColor.COLOR_TONE_NO) {
	    oldColor = colorTone;
	    colorTone = levelColorTone;
	  }
	  else if ( levelFillColor != GeColor.COLOR_NO) {
	    oldColor = fillColor;
	    fillColor = levelFillColor;
	  }
	  break;
        case 1:
	  if ( levelColorTone != GeColor.COLOR_TONE_NO)
	    colorTone = oldColor;
	  else if ( levelFillColor != GeColor.COLOR_NO)
	    fillColor = oldColor;
	  break;
        }
        switch ( levelDirection) {
        case Ge.DIRECTION_UP:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,fillLevel*original_height+Ge.cJBean_Offset,original_width,original_height));
	  else
	    g.setClip(new Rectangle2D.Float(0F,0F,original_width,fillLevel * original_height+Ge.cJBean_Offset));
	  break;
	case Ge.DIRECTION_DOWN:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,0F,original_width,(1-fillLevel)*original_height+Ge.cJBean_Offset));
	  else
	    g.setClip(new Rectangle2D.Float(0F,(1-fillLevel)*original_height+Ge.cJBean_Offset,original_width,original_height));
	  break;
	case Ge.DIRECTION_RIGHT:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(fillLevel*original_width+Ge.cJBean_Offset,0F,original_width,original_height));
	  else
	    g.setClip(new Rectangle2D.Float(0F,0F,fillLevel*width+Ge.cJBean_Offset,height));
	  break;
	case Ge.DIRECTION_LEFT:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,0F,(1-fillLevel)*original_width+Ge.cJBean_Offset,original_height));
	  else
	    g.setClip(new Rectangle2D.Float((1-fillLevel)*original_width+Ge.cJBean_Offset,0F,original_width,original_height));
	  break;
	}
      }
    {
    int fcolor = GeColor.getDrawtype(21, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    g.setColor(GeColor.getColor( fcolor));
    g.fill( shapes[0]);
    if ( shadow != 0) {
    g.setColor(GeColor.shiftColor( fcolor, -2, colorInverse));
    g.fill( shapes[1]);
    g.setColor(GeColor.shiftColor( fcolor, 2, colorInverse));
    g.fill( shapes[2]);
    }
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[0]);
    }
    {
    int fcolor = GeColor.getDrawtype(23, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    g.setColor(GeColor.getColor( fcolor));
    g.fill( shapes[3]);
    if ( shadow != 0) {
    g.setColor(GeColor.shiftColor( fcolor, -2, colorInverse));
    g.fill( shapes[4]);
    g.setColor(GeColor.shiftColor( fcolor, 2, colorInverse));
    g.fill( shapes[5]);
    }
    }
    {
    int fcolor = GeColor.getDrawtype(25, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    g.setColor(GeColor.getColor( fcolor));
    g.fill( shapes[6]);
    if ( shadow != 0) {
    g.setColor(GeColor.shiftColor( fcolor, -2, colorInverse));
    g.fill( shapes[7]);
    g.setColor(GeColor.shiftColor( fcolor, 2, colorInverse));
    g.fill( shapes[8]);
    }
    }
    {
    int fcolor = GeColor.getDrawtype(22, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    g.setColor(GeColor.getColor( fcolor));
    g.fill( shapes[9]);
    if ( shadow != 0) {
    g.setColor(GeColor.shiftColor( fcolor, -2, colorInverse));
    g.fill( shapes[10]);
    g.setColor(GeColor.shiftColor( fcolor, 2, colorInverse));
    g.fill( shapes[11]);
    }
    }
    g.setStroke( new BasicStroke(1F, BasicStroke.CAP_BUTT, BasicStroke.JOIN_ROUND));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[12]);
    {
    int fcolor = GeColor.getDrawtype(23, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    g.setColor(GeColor.getColor( fcolor));
    g.fill( shapes[13]);
    if ( shadow != 0) {
    g.setColor(GeColor.shiftColor( fcolor, -2, colorInverse));
    g.fill( shapes[14]);
    g.setColor(GeColor.shiftColor( fcolor, 2, colorInverse));
    g.fill( shapes[15]);
    }
    }
    {
    int fcolor = GeColor.getDrawtype(25, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    g.setColor(GeColor.getColor( fcolor));
    g.fill( shapes[16]);
    if ( shadow != 0) {
    g.setColor(GeColor.shiftColor( fcolor, -2, colorInverse));
    g.fill( shapes[17]);
    g.setColor(GeColor.shiftColor( fcolor, 2, colorInverse));
    g.fill( shapes[18]);
    }
    }
    {
    int fcolor = GeColor.getDrawtype(22, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    g.setColor(GeColor.getColor( fcolor));
    g.fill( shapes[19]);
    if ( shadow != 0) {
    g.setColor(GeColor.shiftColor( fcolor, -2, colorInverse));
    g.fill( shapes[20]);
    g.setColor(GeColor.shiftColor( fcolor, 2, colorInverse));
    g.fill( shapes[21]);
    }
    }
    g.setStroke( new BasicStroke(2F, BasicStroke.CAP_BUTT, BasicStroke.JOIN_ROUND));
    g.setColor(GeColor.getColor(3, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[22]);
    {
    int fcolor = GeColor.getDrawtype(21, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    if ( shadow != 0) {
    g.setColor(GeColor.shiftColor( fcolor, -2, colorInverse));
    g.fill( shapes[24]);
    g.setColor(GeColor.shiftColor( fcolor, 2, colorInverse));
    g.fill( shapes[25]);
    }
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[23]);
    }
    {
    int fcolor = GeColor.getDrawtype(21, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    if ( shadow != 0) {
    g.setColor(GeColor.shiftColor( fcolor, -2, colorInverse));
    g.fill( shapes[27]);
    g.setColor(GeColor.shiftColor( fcolor, 2, colorInverse));
    g.fill( shapes[28]);
    }
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[26]);
    }
    }
    if ( rounds == 2)
      g.setClip(null);
    g.setTransform(save);
  }
  public Dimension getPreferredSize() { return size;}
  public Dimension getMinimumSize() { return size;}
}
protected class pwr_indsquare extends GeComponent {
  // Dimension size;
  public pwr_indsquare( JopSession session)
  {
    super( session);
    size = new Dimension( 16, 17);
  }
  public int original_width = 16;
  public int original_height = 17;
  Shape[] shapes = new Shape[] { 
    new Rectangle2D.Float(2F, 2F, 12F, 13F),
    new Polygon( new int[] { 2, 14, 12, 4, 4, 2}, new int[] { 2, 2, 4, 4, 13, 15}, 6), 
    new Polygon( new int[] { 14, 14, 12, 12, 4, 2}, new int[] { 15, 2, 4, 13, 13, 15}, 6), 
  };
  public void paintComponent(Graphics g1) {
    animationCount = 1;
    if ( !visible)
      return;
    Graphics2D g = (Graphics2D) g1;
    float width = getWidth();
    float height = getHeight();
    g.setRenderingHint(RenderingHints.KEY_ANTIALIASING,RenderingHints.VALUE_ANTIALIAS_ON);
    if ( 45.0 <= rotate && rotate < 135.0) {
      g.translate( width, 0.0); 
      g.rotate( Math.PI * rotate/180, 0.0, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else if ( 135.0 <= rotate && rotate < 225.0)
    {
      g.rotate( Math.PI * rotate/180, width/2, height/2);
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    }
    else if ( 225.0 <= rotate && rotate < 315.0)
    {
      g.translate( -height, 0.0);
      g.rotate( Math.PI * rotate/180, height, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    if ( (dd.dynType & GeDyn.mDynType_Rotate) != 0 && dd.rotate != 0) {
      g.rotate( Math.PI * dd.rotate/180, 

           (dd.x0 - getX())*original_width/width,
           (dd.y0 - getY()) * original_height / height);
    }
    AffineTransform save = g.getTransform();
    AffineTransform save_tmp;
    int rounds = 1;
    if ( fillLevel != 1F)
      rounds = 2;
    int oldColor = 0;
    for ( int i = 0; i < rounds; i++) {
      if ( rounds == 2) {
        switch ( i) {
        case 0:
	  if ( levelColorTone != GeColor.COLOR_TONE_NO) {
	    oldColor = colorTone;
	    colorTone = levelColorTone;
	  }
	  else if ( levelFillColor != GeColor.COLOR_NO) {
	    oldColor = fillColor;
	    fillColor = levelFillColor;
	  }
	  break;
        case 1:
	  if ( levelColorTone != GeColor.COLOR_TONE_NO)
	    colorTone = oldColor;
	  else if ( levelFillColor != GeColor.COLOR_NO)
	    fillColor = oldColor;
	  break;
        }
        switch ( levelDirection) {
        case Ge.DIRECTION_UP:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,fillLevel*original_height+Ge.cJBean_Offset,original_width,original_height));
	  else
	    g.setClip(new Rectangle2D.Float(0F,0F,original_width,fillLevel * original_height+Ge.cJBean_Offset));
	  break;
	case Ge.DIRECTION_DOWN:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,0F,original_width,(1-fillLevel)*original_height+Ge.cJBean_Offset));
	  else
	    g.setClip(new Rectangle2D.Float(0F,(1-fillLevel)*original_height+Ge.cJBean_Offset,original_width,original_height));
	  break;
	case Ge.DIRECTION_RIGHT:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(fillLevel*original_width+Ge.cJBean_Offset,0F,original_width,original_height));
	  else
	    g.setClip(new Rectangle2D.Float(0F,0F,fillLevel*width+Ge.cJBean_Offset,height));
	  break;
	case Ge.DIRECTION_LEFT:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,0F,(1-fillLevel)*original_width+Ge.cJBean_Offset,original_height));
	  else
	    g.setClip(new Rectangle2D.Float((1-fillLevel)*original_width+Ge.cJBean_Offset,0F,original_width,original_height));
	  break;
	}
      }
    {
    int fcolor = GeColor.getDrawtype(293, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    g.setColor(GeColor.getColor( fcolor));
    g.fill( shapes[0]);
    if ( shadow != 0) {
    g.setColor(GeColor.shiftColor( fcolor, -2, colorInverse));
    g.fill( shapes[1]);
    g.setColor(GeColor.shiftColor( fcolor, 2, colorInverse));
    g.fill( shapes[2]);
    }
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[0]);
    }
    }
    if ( rounds == 2)
      g.setClip(null);
    g.setTransform(save);
  }
  public Dimension getPreferredSize() { return size;}
  public Dimension getMinimumSize() { return size;}
}
protected class pwr_mbsimulate extends GeComponent {
  // Dimension size;
  public pwr_mbsimulate( JopSession session)
  {
    super( session);
    size = new Dimension( 20, 20);
  }
  public int original_width = 20;
  public int original_height = 20;
  Shape[] shapes = new Shape[] { 
    new Rectangle2D.Float(2F, 2F, 16F, 16F),
    new Polygon( new int[] { 2, 18, 16, 4, 4, 2}, new int[] { 2, 2, 4, 4, 16, 18}, 6), 
    new Polygon( new int[] { 18, 18, 16, 16, 4, 2}, new int[] { 18, 2, 4, 16, 16, 18}, 6), 
    new Polygon( new int[] { 12, 7, 6, 14, 13, 5, 13, 14, 6, 7}, new int[] {6, 6, 8, 12, 14, 14, 14, 12, 8, 6}, 10),
  };
  public void paintComponent(Graphics g1) {
    animationCount = 1;
    if ( !visible)
      return;
    Graphics2D g = (Graphics2D) g1;
    float width = getWidth();
    float height = getHeight();
    g.setRenderingHint(RenderingHints.KEY_ANTIALIASING,RenderingHints.VALUE_ANTIALIAS_ON);
    if ( 45.0 <= rotate && rotate < 135.0) {
      g.translate( width, 0.0); 
      g.rotate( Math.PI * rotate/180, 0.0, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else if ( 135.0 <= rotate && rotate < 225.0)
    {
      g.rotate( Math.PI * rotate/180, width/2, height/2);
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    }
    else if ( 225.0 <= rotate && rotate < 315.0)
    {
      g.translate( -height, 0.0);
      g.rotate( Math.PI * rotate/180, height, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    if ( (dd.dynType & GeDyn.mDynType_Rotate) != 0 && dd.rotate != 0) {
      g.rotate( Math.PI * dd.rotate/180, 

           (dd.x0 - getX())*original_width/width,
           (dd.y0 - getY()) * original_height / height);
    }
    AffineTransform save = g.getTransform();
    AffineTransform save_tmp;
    int rounds = 1;
    if ( fillLevel != 1F)
      rounds = 2;
    int oldColor = 0;
    for ( int i = 0; i < rounds; i++) {
      if ( rounds == 2) {
        switch ( i) {
        case 0:
	  if ( levelColorTone != GeColor.COLOR_TONE_NO) {
	    oldColor = colorTone;
	    colorTone = levelColorTone;
	  }
	  else if ( levelFillColor != GeColor.COLOR_NO) {
	    oldColor = fillColor;
	    fillColor = levelFillColor;
	  }
	  break;
        case 1:
	  if ( levelColorTone != GeColor.COLOR_TONE_NO)
	    colorTone = oldColor;
	  else if ( levelFillColor != GeColor.COLOR_NO)
	    fillColor = oldColor;
	  break;
        }
        switch ( levelDirection) {
        case Ge.DIRECTION_UP:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,fillLevel*original_height+Ge.cJBean_Offset,original_width,original_height));
	  else
	    g.setClip(new Rectangle2D.Float(0F,0F,original_width,fillLevel * original_height+Ge.cJBean_Offset));
	  break;
	case Ge.DIRECTION_DOWN:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,0F,original_width,(1-fillLevel)*original_height+Ge.cJBean_Offset));
	  else
	    g.setClip(new Rectangle2D.Float(0F,(1-fillLevel)*original_height+Ge.cJBean_Offset,original_width,original_height));
	  break;
	case Ge.DIRECTION_RIGHT:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(fillLevel*original_width+Ge.cJBean_Offset,0F,original_width,original_height));
	  else
	    g.setClip(new Rectangle2D.Float(0F,0F,fillLevel*width+Ge.cJBean_Offset,height));
	  break;
	case Ge.DIRECTION_LEFT:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,0F,(1-fillLevel)*original_width+Ge.cJBean_Offset,original_height));
	  else
	    g.setClip(new Rectangle2D.Float((1-fillLevel)*original_width+Ge.cJBean_Offset,0F,original_width,original_height));
	  break;
	}
      }
    {
    int fcolor = GeColor.getDrawtype(34, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    g.setColor(GeColor.getColor( fcolor));
    g.fill( shapes[0]);
    g.setColor(GeColor.shiftColor( fcolor, -2, colorInverse));
    g.fill( shapes[1]);
    g.setColor(GeColor.shiftColor( fcolor, 2, colorInverse));
    g.fill( shapes[2]);
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[0]);
    }
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(39, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[3]);
    }
    if ( rounds == 2)
      g.setClip(null);
    g.setTransform(save);
  }
  public Dimension getPreferredSize() { return size;}
  public Dimension getMinimumSize() { return size;}
}
protected class pwr_valuesmall extends GeComponent {
  // Dimension size;
  public pwr_valuesmall( JopSession session)
  {
    super( session);
    size = new Dimension( 50, 18);
    annot1Font = new Font("Helvetica", Font.BOLD, 10);
  }
  int annot1Color = 0;
  public String getAnnot1() { return annot1;}
  public void setAnnot1( String s) { annot1 = s;}
  public void setAnnot1Font( Font font) { annot1Font = font;}
  public Font getAnnot1Font() { return annot1Font;}
  public void setAnnot1Color( int color) { annot1Color = color;}
  public int original_width = 50;
  public int original_height = 18;
  Shape[] shapes = new Shape[] { 
    new Rectangle2D.Float(2F, 2F, 46F, 14F),
    new Polygon( new int[] { 2, 48, 47, 3, 3, 2}, new int[] { 2, 2, 3, 3, 15, 16}, 6), 
    new Polygon( new int[] { 48, 48, 47, 47, 3, 2}, new int[] { 16, 2, 3, 15, 15, 16}, 6), 
  };
  public void paintComponent(Graphics g1) {
    animationCount = 1;
    if ( !visible)
      return;
    Graphics2D g = (Graphics2D) g1;
    float width = getWidth();
    float height = getHeight();
    g.setRenderingHint(RenderingHints.KEY_ANTIALIASING,RenderingHints.VALUE_ANTIALIAS_ON);
    if ( 45.0 <= rotate && rotate < 135.0) {
      g.translate( width, 0.0); 
      g.rotate( Math.PI * rotate/180, 0.0, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else if ( 135.0 <= rotate && rotate < 225.0)
    {
      g.rotate( Math.PI * rotate/180, width/2, height/2);
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    }
    else if ( 225.0 <= rotate && rotate < 315.0)
    {
      g.translate( -height, 0.0);
      g.rotate( Math.PI * rotate/180, height, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    if ( (dd.dynType & GeDyn.mDynType_Rotate) != 0 && dd.rotate != 0) {
      g.rotate( Math.PI * dd.rotate/180, 

           (dd.x0 - getX())*original_width/width,
           (dd.y0 - getY()) * original_height / height);
    }
    AffineTransform save = g.getTransform();
    AffineTransform save_tmp;
    int rounds = 1;
    if ( fillLevel != 1F)
      rounds = 2;
    int oldColor = 0;
    for ( int i = 0; i < rounds; i++) {
      if ( rounds == 2) {
        switch ( i) {
        case 0:
	  if ( levelColorTone != GeColor.COLOR_TONE_NO) {
	    oldColor = colorTone;
	    colorTone = levelColorTone;
	  }
	  else if ( levelFillColor != GeColor.COLOR_NO) {
	    oldColor = fillColor;
	    fillColor = levelFillColor;
	  }
	  break;
        case 1:
	  if ( levelColorTone != GeColor.COLOR_TONE_NO)
	    colorTone = oldColor;
	  else if ( levelFillColor != GeColor.COLOR_NO)
	    fillColor = oldColor;
	  break;
        }
        switch ( levelDirection) {
        case Ge.DIRECTION_UP:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,fillLevel*original_height+Ge.cJBean_Offset,original_width,original_height));
	  else
	    g.setClip(new Rectangle2D.Float(0F,0F,original_width,fillLevel * original_height+Ge.cJBean_Offset));
	  break;
	case Ge.DIRECTION_DOWN:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,0F,original_width,(1-fillLevel)*original_height+Ge.cJBean_Offset));
	  else
	    g.setClip(new Rectangle2D.Float(0F,(1-fillLevel)*original_height+Ge.cJBean_Offset,original_width,original_height));
	  break;
	case Ge.DIRECTION_RIGHT:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(fillLevel*original_width+Ge.cJBean_Offset,0F,original_width,original_height));
	  else
	    g.setClip(new Rectangle2D.Float(0F,0F,fillLevel*width+Ge.cJBean_Offset,height));
	  break;
	case Ge.DIRECTION_LEFT:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,0F,(1-fillLevel)*original_width+Ge.cJBean_Offset,original_height));
	  else
	    g.setClip(new Rectangle2D.Float((1-fillLevel)*original_width+Ge.cJBean_Offset,0F,original_width,original_height));
	  break;
	}
      }
    {
    int fcolor = GeColor.getDrawtype(41, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    g.setColor(GeColor.getColor( fcolor));
    g.fill( shapes[0]);
    if ( shadow != 0) {
    g.setColor(GeColor.shiftColor( fcolor, -2, colorInverse));
    g.fill( shapes[1]);
    g.setColor(GeColor.shiftColor( fcolor, 2, colorInverse));
    g.fill( shapes[2]);
    }
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[0]);
    }
    g.setColor(GeColor.getColor( annot1Color , colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, textColor, dimmed));
    g.setFont( annot1Font);
    save_tmp = g.getTransform();
    g.setRenderingHint(RenderingHints.KEY_ANTIALIASING,RenderingHints.VALUE_ANTIALIAS_OFF);
    g.transform( AffineTransform.getScaleInstance( original_width/width *
      		height/original_height, 1));
    if ( annot1 != null)
      g.drawString( annot1, 7 * original_height / height * width / original_width, 13F);
    g.setTransform( save_tmp);
    }
    if ( rounds == 2)
      g.setClip(null);
    g.setTransform(save);
  }
  public Dimension getPreferredSize() { return size;}
  public Dimension getMinimumSize() { return size;}
}
protected class pwr_indround extends GeComponent {
  // Dimension size;
  public pwr_indround( JopSession session)
  {
    super( session);
    size = new Dimension( 18, 18);
  }
  public int original_width = 18;
  public int original_height = 18;
  Shape[] shapes = new Shape[] { 
    new Arc2D.Float(2F, 2F, 14F, 14F, 0F, 360F, Arc2D.PIE),
    new Arc2D.Float(2F, 2F, 14F, 14F, 35F, 140F, Arc2D.PIE),
    new Arc2D.Float(2F, 2F, 14F, 14F, 215F, 140F, Arc2D.PIE),
    new Arc2D.Float(2F, 2F, 14F, 14F, -5F, 40F, Arc2D.PIE),
    new Arc2D.Float(2F, 2F, 14F, 14F, 175F, 40F, Arc2D.PIE),
    new Arc2D.Float(4.1F, 4.1F, 9.8F, 9.8F, 0F, 360F, Arc2D.PIE),
    new Arc2D.Float(2F, 2F, 14F, 14F, 0F, 360F, Arc2D.PIE),
  };
  public void paintComponent(Graphics g1) {
    animationCount = 1;
    if ( !visible)
      return;
    Graphics2D g = (Graphics2D) g1;
    float width = getWidth();
    float height = getHeight();
    g.setRenderingHint(RenderingHints.KEY_ANTIALIASING,RenderingHints.VALUE_ANTIALIAS_ON);
    if ( 45.0 <= rotate && rotate < 135.0) {
      g.translate( width, 0.0); 
      g.rotate( Math.PI * rotate/180, 0.0, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else if ( 135.0 <= rotate && rotate < 225.0)
    {
      g.rotate( Math.PI * rotate/180, width/2, height/2);
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    }
    else if ( 225.0 <= rotate && rotate < 315.0)
    {
      g.translate( -height, 0.0);
      g.rotate( Math.PI * rotate/180, height, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    if ( (dd.dynType & GeDyn.mDynType_Rotate) != 0 && dd.rotate != 0) {
      g.rotate( Math.PI * dd.rotate/180, 

           (dd.x0 - getX())*original_width/width,
           (dd.y0 - getY()) * original_height / height);
    }
    AffineTransform save = g.getTransform();
    AffineTransform save_tmp;
    int rounds = 1;
    if ( fillLevel != 1F)
      rounds = 2;
    int oldColor = 0;
    for ( int i = 0; i < rounds; i++) {
      if ( rounds == 2) {
        switch ( i) {
        case 0:
	  if ( levelColorTone != GeColor.COLOR_TONE_NO) {
	    oldColor = colorTone;
	    colorTone = levelColorTone;
	  }
	  else if ( levelFillColor != GeColor.COLOR_NO) {
	    oldColor = fillColor;
	    fillColor = levelFillColor;
	  }
	  break;
        case 1:
	  if ( levelColorTone != GeColor.COLOR_TONE_NO)
	    colorTone = oldColor;
	  else if ( levelFillColor != GeColor.COLOR_NO)
	    fillColor = oldColor;
	  break;
        }
        switch ( levelDirection) {
        case Ge.DIRECTION_UP:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,fillLevel*original_height+Ge.cJBean_Offset,original_width,original_height));
	  else
	    g.setClip(new Rectangle2D.Float(0F,0F,original_width,fillLevel * original_height+Ge.cJBean_Offset));
	  break;
	case Ge.DIRECTION_DOWN:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,0F,original_width,(1-fillLevel)*original_height+Ge.cJBean_Offset));
	  else
	    g.setClip(new Rectangle2D.Float(0F,(1-fillLevel)*original_height+Ge.cJBean_Offset,original_width,original_height));
	  break;
	case Ge.DIRECTION_RIGHT:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(fillLevel*original_width+Ge.cJBean_Offset,0F,original_width,original_height));
	  else
	    g.setClip(new Rectangle2D.Float(0F,0F,fillLevel*width+Ge.cJBean_Offset,height));
	  break;
	case Ge.DIRECTION_LEFT:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,0F,(1-fillLevel)*original_width+Ge.cJBean_Offset,original_height));
	  else
	    g.setClip(new Rectangle2D.Float((1-fillLevel)*original_width+Ge.cJBean_Offset,0F,original_width,original_height));
	  break;
	}
      }
    if ( shadow == 0) {
    ((Arc2D)shapes[0]).setArcType(Arc2D.PIE);
    g.setColor(GeColor.getColor(293, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes[0]);
    ((Arc2D)shapes[0]).setArcType(Arc2D.OPEN);
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[0]);
    } else {
    ((Arc2D)shapes[1]).setArcType(Arc2D.PIE);
    ((Arc2D)shapes[2]).setArcType(Arc2D.PIE);
    ((Arc2D)shapes[3]).setArcType(Arc2D.PIE);
    ((Arc2D)shapes[4]).setArcType(Arc2D.PIE);
    ((Arc2D)shapes[5]).setArcType(Arc2D.PIE);
    {
    int fcolor = GeColor.getDrawtype(293, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    g.setColor(GeColor.shiftColor( fcolor, -2, colorInverse));
    g.fill( shapes[1]);
    g.setColor(GeColor.shiftColor( fcolor, 2, colorInverse));
    g.fill( shapes[2]);
    g.setColor(GeColor.getColor( fcolor));
    g.fill( shapes[3]);
    g.fill( shapes[4]);
    g.fill( shapes[5]);
    }
    ((Arc2D)shapes[6]).setArcType(Arc2D.OPEN);
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[6]);
    }
    }
    if ( rounds == 2)
      g.setClip(null);
    g.setTransform(save);
  }
  public Dimension getPreferredSize() { return size;}
  public Dimension getMinimumSize() { return size;}
}
protected class JopSmallbutton extends GeComponent {
  // Dimension size;
  public JopSmallbutton( JopSession session)
  {
    super( session);
    size = new Dimension( 47, 21);
    annot1Font = new Font("Helvetica", Font.BOLD, 12);
  }
  int annot1Color = 0;
  public String getAnnot1() { return annot1;}
  public void setAnnot1( String s) { annot1 = s;}
  public void setAnnot1Font( Font font) { annot1Font = font;}
  public Font getAnnot1Font() { return annot1Font;}
  public void setAnnot1Color( int color) { annot1Color = color;}
  public int original_width = 47;
  public int original_height = 21;
  Shape[] shapes = new Shape[] { 
    new Rectangle2D.Float(2F, 2F, 44F, 17F),
    new Polygon( new int[] { 2, 46, 43, 5, 5, 2}, new int[] { 2, 2, 5, 5, 16, 19}, 6), 
    new Polygon( new int[] { 46, 46, 43, 43, 5, 2}, new int[] { 19, 2, 5, 16, 16, 19}, 6), 
  };
  public void paintComponent(Graphics g1) {
    animationCount = 1;
    if ( !visible)
      return;
    Graphics2D g = (Graphics2D) g1;
    float width = getWidth();
    float height = getHeight();
    g.setRenderingHint(RenderingHints.KEY_ANTIALIASING,RenderingHints.VALUE_ANTIALIAS_ON);
    if ( 45.0 <= rotate && rotate < 135.0) {
      g.translate( width, 0.0); 
      g.rotate( Math.PI * rotate/180, 0.0, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else if ( 135.0 <= rotate && rotate < 225.0)
    {
      g.rotate( Math.PI * rotate/180, width/2, height/2);
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    }
    else if ( 225.0 <= rotate && rotate < 315.0)
    {
      g.translate( -height, 0.0);
      g.rotate( Math.PI * rotate/180, height, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    if ( (dd.dynType & GeDyn.mDynType_Rotate) != 0 && dd.rotate != 0) {
      g.rotate( Math.PI * dd.rotate/180, 

           (dd.x0 - getX())*original_width/width,
           (dd.y0 - getY()) * original_height / height);
    }
    AffineTransform save = g.getTransform();
    AffineTransform save_tmp;
    int rounds = 1;
    if ( fillLevel != 1F)
      rounds = 2;
    int oldColor = 0;
    for ( int i = 0; i < rounds; i++) {
      if ( rounds == 2) {
        switch ( i) {
        case 0:
	  if ( levelColorTone != GeColor.COLOR_TONE_NO) {
	    oldColor = colorTone;
	    colorTone = levelColorTone;
	  }
	  else if ( levelFillColor != GeColor.COLOR_NO) {
	    oldColor = fillColor;
	    fillColor = levelFillColor;
	  }
	  break;
        case 1:
	  if ( levelColorTone != GeColor.COLOR_TONE_NO)
	    colorTone = oldColor;
	  else if ( levelFillColor != GeColor.COLOR_NO)
	    fillColor = oldColor;
	  break;
        }
        switch ( levelDirection) {
        case Ge.DIRECTION_UP:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,fillLevel*original_height+Ge.cJBean_Offset,original_width,original_height));
	  else
	    g.setClip(new Rectangle2D.Float(0F,0F,original_width,fillLevel * original_height+Ge.cJBean_Offset));
	  break;
	case Ge.DIRECTION_DOWN:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,0F,original_width,(1-fillLevel)*original_height+Ge.cJBean_Offset));
	  else
	    g.setClip(new Rectangle2D.Float(0F,(1-fillLevel)*original_height+Ge.cJBean_Offset,original_width,original_height));
	  break;
	case Ge.DIRECTION_RIGHT:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(fillLevel*original_width+Ge.cJBean_Offset,0F,original_width,original_height));
	  else
	    g.setClip(new Rectangle2D.Float(0F,0F,fillLevel*width+Ge.cJBean_Offset,height));
	  break;
	case Ge.DIRECTION_LEFT:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,0F,(1-fillLevel)*original_width+Ge.cJBean_Offset,original_height));
	  else
	    g.setClip(new Rectangle2D.Float((1-fillLevel)*original_width+Ge.cJBean_Offset,0F,original_width,original_height));
	  break;
	}
      }
    {
    int fcolor = GeColor.getDrawtype(102, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    g.setColor(GeColor.getColor( fcolor));
    g.fill( shapes[0]);
    g.setColor(GeColor.shiftColor( fcolor, -2, colorInverse));
    g.fill( shapes[1]);
    g.setColor(GeColor.shiftColor( fcolor, 2, colorInverse));
    g.fill( shapes[2]);
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[0]);
    }
    g.setColor(GeColor.getColor( annot1Color , colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, textColor, dimmed));
    g.setFont( annot1Font);
    save_tmp = g.getTransform();
    g.setRenderingHint(RenderingHints.KEY_ANTIALIASING,RenderingHints.VALUE_ANTIALIAS_OFF);
    g.transform( AffineTransform.getScaleInstance( original_width/width *
      		height/original_height, 1));
    if ( annot1 != null)
      g.drawString( annot1, 6 * original_height / height * width / original_width, 15F);
    g.setTransform( save_tmp);
    }
    if ( rounds == 2)
      g.setClip(null);
    g.setTransform(save);
  }
  public Dimension getPreferredSize() { return size;}
  public Dimension getMinimumSize() { return size;}
}
protected class pwr_menubar extends GeComponent {
  // Dimension size;
  public pwr_menubar( JopSession session)
  {
    super( session);
    size = new Dimension( 554, 24);
  }
  public int original_width = 554;
  public int original_height = 24;
  Shape[] shapes = new Shape[] { 
    new Rectangle2D.Float(2F, 2F, 550F, 20F),
    new Polygon( new int[] { 2, 552, 549, 5, 5, 2}, new int[] { 2, 2, 5, 5, 19, 22}, 6), 
    new Polygon( new int[] { 552, 552, 549, 549, 5, 2}, new int[] { 22, 2, 5, 19, 19, 22}, 6), 
  };
  public void paintComponent(Graphics g1) {
    animationCount = 1;
    if ( !visible)
      return;
    Graphics2D g = (Graphics2D) g1;
    float width = getWidth();
    float height = getHeight();
    g.setRenderingHint(RenderingHints.KEY_ANTIALIASING,RenderingHints.VALUE_ANTIALIAS_ON);
    if ( 45.0 <= rotate && rotate < 135.0) {
      g.translate( width, 0.0); 
      g.rotate( Math.PI * rotate/180, 0.0, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else if ( 135.0 <= rotate && rotate < 225.0)
    {
      g.rotate( Math.PI * rotate/180, width/2, height/2);
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    }
    else if ( 225.0 <= rotate && rotate < 315.0)
    {
      g.translate( -height, 0.0);
      g.rotate( Math.PI * rotate/180, height, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    if ( (dd.dynType & GeDyn.mDynType_Rotate) != 0 && dd.rotate != 0) {
      g.rotate( Math.PI * dd.rotate/180, 

           (dd.x0 - getX())*original_width/width,
           (dd.y0 - getY()) * original_height / height);
    }
    AffineTransform save = g.getTransform();
    AffineTransform save_tmp;
    int rounds = 1;
    if ( fillLevel != 1F)
      rounds = 2;
    int oldColor = 0;
    for ( int i = 0; i < rounds; i++) {
      if ( rounds == 2) {
        switch ( i) {
        case 0:
	  if ( levelColorTone != GeColor.COLOR_TONE_NO) {
	    oldColor = colorTone;
	    colorTone = levelColorTone;
	  }
	  else if ( levelFillColor != GeColor.COLOR_NO) {
	    oldColor = fillColor;
	    fillColor = levelFillColor;
	  }
	  break;
        case 1:
	  if ( levelColorTone != GeColor.COLOR_TONE_NO)
	    colorTone = oldColor;
	  else if ( levelFillColor != GeColor.COLOR_NO)
	    fillColor = oldColor;
	  break;
        }
        switch ( levelDirection) {
        case Ge.DIRECTION_UP:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,fillLevel*original_height+Ge.cJBean_Offset,original_width,original_height));
	  else
	    g.setClip(new Rectangle2D.Float(0F,0F,original_width,fillLevel * original_height+Ge.cJBean_Offset));
	  break;
	case Ge.DIRECTION_DOWN:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,0F,original_width,(1-fillLevel)*original_height+Ge.cJBean_Offset));
	  else
	    g.setClip(new Rectangle2D.Float(0F,(1-fillLevel)*original_height+Ge.cJBean_Offset,original_width,original_height));
	  break;
	case Ge.DIRECTION_RIGHT:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(fillLevel*original_width+Ge.cJBean_Offset,0F,original_width,original_height));
	  else
	    g.setClip(new Rectangle2D.Float(0F,0F,fillLevel*width+Ge.cJBean_Offset,height));
	  break;
	case Ge.DIRECTION_LEFT:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,0F,(1-fillLevel)*original_width+Ge.cJBean_Offset,original_height));
	  else
	    g.setClip(new Rectangle2D.Float((1-fillLevel)*original_width+Ge.cJBean_Offset,0F,original_width,original_height));
	  break;
	}
      }
    {
    int fcolor = GeColor.getDrawtype(31, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    g.setColor(GeColor.getColor( fcolor));
    g.fill( shapes[0]);
    if ( shadow != 0) {
    g.setColor(GeColor.shiftColor( fcolor, -2, colorInverse));
    g.fill( shapes[1]);
    g.setColor(GeColor.shiftColor( fcolor, 2, colorInverse));
    g.fill( shapes[2]);
    }
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[0]);
    }
    }
    if ( rounds == 2)
      g.setClip(null);
    g.setTransform(save);
  }
  public Dimension getPreferredSize() { return size;}
  public Dimension getMinimumSize() { return size;}
}
protected class pwr_pulldownmenu extends GeComponent {
  // Dimension size;
  public pwr_pulldownmenu( JopSession session)
  {
    super( session);
    size = new Dimension( 64, 24);
    annot1Font = new Font("Helvetica", Font.BOLD, 12);
  }
  int annot1Color = 0;
  public String getAnnot1() { return annot1;}
  public void setAnnot1( String s) { annot1 = s;}
  public void setAnnot1Font( Font font) { annot1Font = font;}
  public Font getAnnot1Font() { return annot1Font;}
  public void setAnnot1Color( int color) { annot1Color = color;}
  public int original_width = 64;
  public int original_height = 24;
  Shape[] shapes = new Shape[] { 
    new Rectangle2D.Float(3F, 5F, 59F, 14F),
    new Polygon( new int[] { 3, 62, 62, 3, 3}, new int[] {19, 19, 22, 22, 19}, 5),
    new Polygon( new int[] { 3, 3, 62, 62, 2}, new int[] {2, 5, 5, 2, 2}, 5),
    new Line2D.Float( 62F, 22F, 2F, 22F),
    new Line2D.Float( 62F, 2F, 2F, 2F),
  };
  public void paintComponent(Graphics g1) {
    animationCount = 1;
    if ( !visible)
      return;
    Graphics2D g = (Graphics2D) g1;
    float width = getWidth();
    float height = getHeight();
    g.setRenderingHint(RenderingHints.KEY_ANTIALIASING,RenderingHints.VALUE_ANTIALIAS_ON);
    if ( 45.0 <= rotate && rotate < 135.0) {
      g.translate( width, 0.0); 
      g.rotate( Math.PI * rotate/180, 0.0, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else if ( 135.0 <= rotate && rotate < 225.0)
    {
      g.rotate( Math.PI * rotate/180, width/2, height/2);
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    }
    else if ( 225.0 <= rotate && rotate < 315.0)
    {
      g.translate( -height, 0.0);
      g.rotate( Math.PI * rotate/180, height, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    if ( (dd.dynType & GeDyn.mDynType_Rotate) != 0 && dd.rotate != 0) {
      g.rotate( Math.PI * dd.rotate/180, 

           (dd.x0 - getX())*original_width/width,
           (dd.y0 - getY()) * original_height / height);
    }
    AffineTransform save = g.getTransform();
    AffineTransform save_tmp;
    int rounds = 1;
    if ( fillLevel != 1F)
      rounds = 2;
    int oldColor = 0;
    for ( int i = 0; i < rounds; i++) {
      if ( rounds == 2) {
        switch ( i) {
        case 0:
	  if ( levelColorTone != GeColor.COLOR_TONE_NO) {
	    oldColor = colorTone;
	    colorTone = levelColorTone;
	  }
	  else if ( levelFillColor != GeColor.COLOR_NO) {
	    oldColor = fillColor;
	    fillColor = levelFillColor;
	  }
	  break;
        case 1:
	  if ( levelColorTone != GeColor.COLOR_TONE_NO)
	    colorTone = oldColor;
	  else if ( levelFillColor != GeColor.COLOR_NO)
	    fillColor = oldColor;
	  break;
        }
        switch ( levelDirection) {
        case Ge.DIRECTION_UP:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,fillLevel*original_height+Ge.cJBean_Offset,original_width,original_height));
	  else
	    g.setClip(new Rectangle2D.Float(0F,0F,original_width,fillLevel * original_height+Ge.cJBean_Offset));
	  break;
	case Ge.DIRECTION_DOWN:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,0F,original_width,(1-fillLevel)*original_height+Ge.cJBean_Offset));
	  else
	    g.setClip(new Rectangle2D.Float(0F,(1-fillLevel)*original_height+Ge.cJBean_Offset,original_width,original_height));
	  break;
	case Ge.DIRECTION_RIGHT:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(fillLevel*original_width+Ge.cJBean_Offset,0F,original_width,original_height));
	  else
	    g.setClip(new Rectangle2D.Float(0F,0F,fillLevel*width+Ge.cJBean_Offset,height));
	  break;
	case Ge.DIRECTION_LEFT:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,0F,(1-fillLevel)*original_width+Ge.cJBean_Offset,original_height));
	  else
	    g.setClip(new Rectangle2D.Float((1-fillLevel)*original_width+Ge.cJBean_Offset,0F,original_width,original_height));
	  break;
	}
      }
    {
    int fcolor = GeColor.getDrawtype(31, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    g.setColor(GeColor.getColor( fcolor));
    g.fill( shapes[0]);
    }
    if ( shadow != 0) {
      int fcolor = GeColor.getDrawtype(31, colorTone,
                   colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
      g.setColor(GeColor.shiftColor( fcolor,2, colorInverse));
    }
    else
      g.setColor(GeColor.getColor(31, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes[1]);
    if ( shadow != 0) {
      int fcolor = GeColor.getDrawtype(31, colorTone,
                   colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
      g.setColor(GeColor.shiftColor(fcolor,-2, colorInverse));
    }
    else
      g.setColor(GeColor.getColor(31, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes[2]);
    g.setColor(GeColor.getColor( annot1Color , colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, textColor, dimmed));
    g.setFont( annot1Font);
    save_tmp = g.getTransform();
    g.setRenderingHint(RenderingHints.KEY_ANTIALIASING,RenderingHints.VALUE_ANTIALIAS_OFF);
    g.transform( AffineTransform.getScaleInstance( original_width/width *
      		height/original_height, 1));
    if ( annot1 != null)
      g.drawString( annot1, 7 * original_height / height * width / original_width, 17F);
    g.setTransform( save_tmp);
    g.setStroke( new BasicStroke(1F, BasicStroke.CAP_BUTT, BasicStroke.JOIN_ROUND));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[3]);
    g.setStroke( new BasicStroke(1F, BasicStroke.CAP_BUTT, BasicStroke.JOIN_ROUND));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[4]);
    }
    if ( rounds == 2)
      g.setClip(null);
    g.setTransform(save);
  }
  public Dimension getPreferredSize() { return size;}
  public Dimension getMinimumSize() { return size;}
}
protected class pwr_valuelong extends GeComponent {
  // Dimension size;
  public pwr_valuelong( JopSession session)
  {
    super( session);
    size = new Dimension( 536, 18);
    annot1Font = new Font("Helvetica", Font.BOLD, 10);
  }
  int annot1Color = 0;
  public String getAnnot1() { return annot1;}
  public void setAnnot1( String s) { annot1 = s;}
  public void setAnnot1Font( Font font) { annot1Font = font;}
  public Font getAnnot1Font() { return annot1Font;}
  public void setAnnot1Color( int color) { annot1Color = color;}
  public int original_width = 536;
  public int original_height = 18;
  Shape[] shapes = new Shape[] { 
    new Rectangle2D.Float(2F, 2F, 532F, 14F),
    new Polygon( new int[] { 2, 534, 533, 3, 3, 2}, new int[] { 2, 2, 3, 3, 15, 16}, 6), 
    new Polygon( new int[] { 534, 534, 533, 533, 3, 2}, new int[] { 16, 2, 3, 15, 15, 16}, 6), 
  };
  public void paintComponent(Graphics g1) {
    animationCount = 1;
    if ( !visible)
      return;
    Graphics2D g = (Graphics2D) g1;
    float width = getWidth();
    float height = getHeight();
    g.setRenderingHint(RenderingHints.KEY_ANTIALIASING,RenderingHints.VALUE_ANTIALIAS_ON);
    if ( 45.0 <= rotate && rotate < 135.0) {
      g.translate( width, 0.0); 
      g.rotate( Math.PI * rotate/180, 0.0, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else if ( 135.0 <= rotate && rotate < 225.0)
    {
      g.rotate( Math.PI * rotate/180, width/2, height/2);
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    }
    else if ( 225.0 <= rotate && rotate < 315.0)
    {
      g.translate( -height, 0.0);
      g.rotate( Math.PI * rotate/180, height, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    if ( (dd.dynType & GeDyn.mDynType_Rotate) != 0 && dd.rotate != 0) {
      g.rotate( Math.PI * dd.rotate/180, 

           (dd.x0 - getX())*original_width/width,
           (dd.y0 - getY()) * original_height / height);
    }
    AffineTransform save = g.getTransform();
    AffineTransform save_tmp;
    int rounds = 1;
    if ( fillLevel != 1F)
      rounds = 2;
    int oldColor = 0;
    for ( int i = 0; i < rounds; i++) {
      if ( rounds == 2) {
        switch ( i) {
        case 0:
	  if ( levelColorTone != GeColor.COLOR_TONE_NO) {
	    oldColor = colorTone;
	    colorTone = levelColorTone;
	  }
	  else if ( levelFillColor != GeColor.COLOR_NO) {
	    oldColor = fillColor;
	    fillColor = levelFillColor;
	  }
	  break;
        case 1:
	  if ( levelColorTone != GeColor.COLOR_TONE_NO)
	    colorTone = oldColor;
	  else if ( levelFillColor != GeColor.COLOR_NO)
	    fillColor = oldColor;
	  break;
        }
        switch ( levelDirection) {
        case Ge.DIRECTION_UP:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,fillLevel*original_height+Ge.cJBean_Offset,original_width,original_height));
	  else
	    g.setClip(new Rectangle2D.Float(0F,0F,original_width,fillLevel * original_height+Ge.cJBean_Offset));
	  break;
	case Ge.DIRECTION_DOWN:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,0F,original_width,(1-fillLevel)*original_height+Ge.cJBean_Offset));
	  else
	    g.setClip(new Rectangle2D.Float(0F,(1-fillLevel)*original_height+Ge.cJBean_Offset,original_width,original_height));
	  break;
	case Ge.DIRECTION_RIGHT:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(fillLevel*original_width+Ge.cJBean_Offset,0F,original_width,original_height));
	  else
	    g.setClip(new Rectangle2D.Float(0F,0F,fillLevel*width+Ge.cJBean_Offset,height));
	  break;
	case Ge.DIRECTION_LEFT:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,0F,(1-fillLevel)*original_width+Ge.cJBean_Offset,original_height));
	  else
	    g.setClip(new Rectangle2D.Float((1-fillLevel)*original_width+Ge.cJBean_Offset,0F,original_width,original_height));
	  break;
	}
      }
    {
    int fcolor = GeColor.getDrawtype(41, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    g.setColor(GeColor.getColor( fcolor));
    g.fill( shapes[0]);
    if ( shadow != 0) {
    g.setColor(GeColor.shiftColor( fcolor, -2, colorInverse));
    g.fill( shapes[1]);
    g.setColor(GeColor.shiftColor( fcolor, 2, colorInverse));
    g.fill( shapes[2]);
    }
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[0]);
    }
    g.setColor(GeColor.getColor( annot1Color , colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, textColor, dimmed));
    g.setFont( annot1Font);
    save_tmp = g.getTransform();
    g.setRenderingHint(RenderingHints.KEY_ANTIALIASING,RenderingHints.VALUE_ANTIALIAS_OFF);
    g.transform( AffineTransform.getScaleInstance( original_width/width *
      		height/original_height, 1));
    if ( annot1 != null)
      g.drawString( annot1, 7 * original_height / height * width / original_width, 13F);
    g.setTransform( save_tmp);
    }
    if ( rounds == 2)
      g.setClip(null);
    g.setTransform(save);
  }
  public Dimension getPreferredSize() { return size;}
  public Dimension getMinimumSize() { return size;}
}
protected class pwr_mbopenobject extends GeComponent {
  // Dimension size;
  public pwr_mbopenobject( JopSession session)
  {
    super( session);
    size = new Dimension( 20, 20);
  }
  public int original_width = 20;
  public int original_height = 20;
  Shape[] shapes = new Shape[] { 
    new Rectangle2D.Float(2F, 2F, 16F, 16F),
    new Polygon( new int[] { 2, 18, 16, 4, 4, 2}, new int[] { 2, 2, 4, 4, 16, 18}, 6), 
    new Polygon( new int[] { 18, 18, 16, 16, 4, 2}, new int[] { 18, 2, 4, 16, 16, 18}, 6), 
    new Rectangle2D.Float(6F, 6F, 8F, 8F),
    new Polygon( new int[] { 6, 14, 14, 6, 6, 6}, new int[] { 6, 6, 6, 6, 14, 14}, 6), 
    new Polygon( new int[] { 14, 14, 14, 14, 6, 6}, new int[] { 14, 6, 6, 14, 14, 14}, 6), 
    new Line2D.Float( 14F, 8F, 6F, 8F),
  };
  public void paintComponent(Graphics g1) {
    animationCount = 1;
    if ( !visible)
      return;
    Graphics2D g = (Graphics2D) g1;
    float width = getWidth();
    float height = getHeight();
    g.setRenderingHint(RenderingHints.KEY_ANTIALIASING,RenderingHints.VALUE_ANTIALIAS_ON);
    if ( 45.0 <= rotate && rotate < 135.0) {
      g.translate( width, 0.0); 
      g.rotate( Math.PI * rotate/180, 0.0, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else if ( 135.0 <= rotate && rotate < 225.0)
    {
      g.rotate( Math.PI * rotate/180, width/2, height/2);
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    }
    else if ( 225.0 <= rotate && rotate < 315.0)
    {
      g.translate( -height, 0.0);
      g.rotate( Math.PI * rotate/180, height, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    if ( (dd.dynType & GeDyn.mDynType_Rotate) != 0 && dd.rotate != 0) {
      g.rotate( Math.PI * dd.rotate/180, 

           (dd.x0 - getX())*original_width/width,
           (dd.y0 - getY()) * original_height / height);
    }
    AffineTransform save = g.getTransform();
    AffineTransform save_tmp;
    int rounds = 1;
    if ( fillLevel != 1F)
      rounds = 2;
    int oldColor = 0;
    for ( int i = 0; i < rounds; i++) {
      if ( rounds == 2) {
        switch ( i) {
        case 0:
	  if ( levelColorTone != GeColor.COLOR_TONE_NO) {
	    oldColor = colorTone;
	    colorTone = levelColorTone;
	  }
	  else if ( levelFillColor != GeColor.COLOR_NO) {
	    oldColor = fillColor;
	    fillColor = levelFillColor;
	  }
	  break;
        case 1:
	  if ( levelColorTone != GeColor.COLOR_TONE_NO)
	    colorTone = oldColor;
	  else if ( levelFillColor != GeColor.COLOR_NO)
	    fillColor = oldColor;
	  break;
        }
        switch ( levelDirection) {
        case Ge.DIRECTION_UP:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,fillLevel*original_height+Ge.cJBean_Offset,original_width,original_height));
	  else
	    g.setClip(new Rectangle2D.Float(0F,0F,original_width,fillLevel * original_height+Ge.cJBean_Offset));
	  break;
	case Ge.DIRECTION_DOWN:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,0F,original_width,(1-fillLevel)*original_height+Ge.cJBean_Offset));
	  else
	    g.setClip(new Rectangle2D.Float(0F,(1-fillLevel)*original_height+Ge.cJBean_Offset,original_width,original_height));
	  break;
	case Ge.DIRECTION_RIGHT:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(fillLevel*original_width+Ge.cJBean_Offset,0F,original_width,original_height));
	  else
	    g.setClip(new Rectangle2D.Float(0F,0F,fillLevel*width+Ge.cJBean_Offset,height));
	  break;
	case Ge.DIRECTION_LEFT:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,0F,(1-fillLevel)*original_width+Ge.cJBean_Offset,original_height));
	  else
	    g.setClip(new Rectangle2D.Float((1-fillLevel)*original_width+Ge.cJBean_Offset,0F,original_width,original_height));
	  break;
	}
      }
    {
    int fcolor = GeColor.getDrawtype(34, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    g.setColor(GeColor.getColor( fcolor));
    g.fill( shapes[0]);
    g.setColor(GeColor.shiftColor( fcolor, -2, colorInverse));
    g.fill( shapes[1]);
    g.setColor(GeColor.shiftColor( fcolor, 2, colorInverse));
    g.fill( shapes[2]);
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[0]);
    }
    {
    int fcolor = GeColor.getDrawtype(38, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    if ( shadow != 0) {
    g.setColor(GeColor.shiftColor( fcolor, -2, colorInverse));
    g.fill( shapes[4]);
    g.setColor(GeColor.shiftColor( fcolor, 2, colorInverse));
    g.fill( shapes[5]);
    }
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(39, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[3]);
    }
    g.setStroke( new BasicStroke(1F, BasicStroke.CAP_BUTT, BasicStroke.JOIN_ROUND));
    g.setColor(GeColor.getColor(39, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[6]);
    }
    if ( rounds == 2)
      g.setClip(null);
    g.setTransform(save);
  }
  public Dimension getPreferredSize() { return size;}
  public Dimension getMinimumSize() { return size;}
}
protected class pwr_mbtrend extends GeComponent {
  // Dimension size;
  public pwr_mbtrend( JopSession session)
  {
    super( session);
    size = new Dimension( 20, 20);
  }
  public int original_width = 20;
  public int original_height = 20;
  Shape[] shapes = new Shape[] { 
    new Rectangle2D.Float(2F, 2F, 16F, 16F),
    new Polygon( new int[] { 2, 18, 16, 4, 4, 2}, new int[] { 2, 2, 4, 4, 16, 18}, 6), 
    new Polygon( new int[] { 18, 18, 16, 16, 4, 2}, new int[] { 18, 2, 4, 16, 16, 18}, 6), 
    new Line2D.Float( 14F, 14F, 6F, 14F),
    new Line2D.Float( 14F, 6F, 14F, 14F),
    new Polygon( new int[] { 14, 12, 10, 7, 5, 7, 10, 12}, new int[] {11, 11, 9, 11, 9, 11, 9, 11}, 8),
  };
  public void paintComponent(Graphics g1) {
    animationCount = 1;
    if ( !visible)
      return;
    Graphics2D g = (Graphics2D) g1;
    float width = getWidth();
    float height = getHeight();
    g.setRenderingHint(RenderingHints.KEY_ANTIALIASING,RenderingHints.VALUE_ANTIALIAS_ON);
    if ( 45.0 <= rotate && rotate < 135.0) {
      g.translate( width, 0.0); 
      g.rotate( Math.PI * rotate/180, 0.0, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else if ( 135.0 <= rotate && rotate < 225.0)
    {
      g.rotate( Math.PI * rotate/180, width/2, height/2);
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    }
    else if ( 225.0 <= rotate && rotate < 315.0)
    {
      g.translate( -height, 0.0);
      g.rotate( Math.PI * rotate/180, height, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    if ( (dd.dynType & GeDyn.mDynType_Rotate) != 0 && dd.rotate != 0) {
      g.rotate( Math.PI * dd.rotate/180, 

           (dd.x0 - getX())*original_width/width,
           (dd.y0 - getY()) * original_height / height);
    }
    AffineTransform save = g.getTransform();
    AffineTransform save_tmp;
    int rounds = 1;
    if ( fillLevel != 1F)
      rounds = 2;
    int oldColor = 0;
    for ( int i = 0; i < rounds; i++) {
      if ( rounds == 2) {
        switch ( i) {
        case 0:
	  if ( levelColorTone != GeColor.COLOR_TONE_NO) {
	    oldColor = colorTone;
	    colorTone = levelColorTone;
	  }
	  else if ( levelFillColor != GeColor.COLOR_NO) {
	    oldColor = fillColor;
	    fillColor = levelFillColor;
	  }
	  break;
        case 1:
	  if ( levelColorTone != GeColor.COLOR_TONE_NO)
	    colorTone = oldColor;
	  else if ( levelFillColor != GeColor.COLOR_NO)
	    fillColor = oldColor;
	  break;
        }
        switch ( levelDirection) {
        case Ge.DIRECTION_UP:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,fillLevel*original_height+Ge.cJBean_Offset,original_width,original_height));
	  else
	    g.setClip(new Rectangle2D.Float(0F,0F,original_width,fillLevel * original_height+Ge.cJBean_Offset));
	  break;
	case Ge.DIRECTION_DOWN:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,0F,original_width,(1-fillLevel)*original_height+Ge.cJBean_Offset));
	  else
	    g.setClip(new Rectangle2D.Float(0F,(1-fillLevel)*original_height+Ge.cJBean_Offset,original_width,original_height));
	  break;
	case Ge.DIRECTION_RIGHT:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(fillLevel*original_width+Ge.cJBean_Offset,0F,original_width,original_height));
	  else
	    g.setClip(new Rectangle2D.Float(0F,0F,fillLevel*width+Ge.cJBean_Offset,height));
	  break;
	case Ge.DIRECTION_LEFT:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,0F,(1-fillLevel)*original_width+Ge.cJBean_Offset,original_height));
	  else
	    g.setClip(new Rectangle2D.Float((1-fillLevel)*original_width+Ge.cJBean_Offset,0F,original_width,original_height));
	  break;
	}
      }
    {
    int fcolor = GeColor.getDrawtype(34, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    g.setColor(GeColor.getColor( fcolor));
    g.fill( shapes[0]);
    g.setColor(GeColor.shiftColor( fcolor, -2, colorInverse));
    g.fill( shapes[1]);
    g.setColor(GeColor.shiftColor( fcolor, 2, colorInverse));
    g.fill( shapes[2]);
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[0]);
    }
    g.setStroke( new BasicStroke(1F, BasicStroke.CAP_BUTT, BasicStroke.JOIN_ROUND));
    g.setColor(GeColor.getColor(39, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[3]);
    g.setStroke( new BasicStroke(1F, BasicStroke.CAP_BUTT, BasicStroke.JOIN_ROUND));
    g.setColor(GeColor.getColor(39, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[4]);
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(39, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[5]);
    }
    if ( rounds == 2)
      g.setClip(null);
    g.setTransform(save);
  }
  public Dimension getPreferredSize() { return size;}
  public Dimension getMinimumSize() { return size;}
}
protected class pwr_mbfast extends GeComponent {
  // Dimension size;
  public pwr_mbfast( JopSession session)
  {
    super( session);
    size = new Dimension( 20, 20);
  }
  public int original_width = 20;
  public int original_height = 20;
  Shape[] shapes = new Shape[] { 
    new Rectangle2D.Float(2F, 2F, 16F, 16F),
    new Polygon( new int[] { 2, 18, 16, 4, 4, 2}, new int[] { 2, 2, 4, 4, 16, 18}, 6), 
    new Polygon( new int[] { 18, 18, 16, 16, 4, 2}, new int[] { 18, 2, 4, 16, 16, 18}, 6), 
    new Line2D.Float( 14F, 14F, 6F, 14F),
    new Line2D.Float( 6F, 6F, 6F, 14F),
    new Polygon( new int[] { 6, 8, 11, 13, 15, 13, 11, 8}, new int[] {10, 9, 12, 11, 12, 11, 12, 9}, 8),
  };
  public void paintComponent(Graphics g1) {
    animationCount = 1;
    if ( !visible)
      return;
    Graphics2D g = (Graphics2D) g1;
    float width = getWidth();
    float height = getHeight();
    g.setRenderingHint(RenderingHints.KEY_ANTIALIASING,RenderingHints.VALUE_ANTIALIAS_ON);
    if ( 45.0 <= rotate && rotate < 135.0) {
      g.translate( width, 0.0); 
      g.rotate( Math.PI * rotate/180, 0.0, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else if ( 135.0 <= rotate && rotate < 225.0)
    {
      g.rotate( Math.PI * rotate/180, width/2, height/2);
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    }
    else if ( 225.0 <= rotate && rotate < 315.0)
    {
      g.translate( -height, 0.0);
      g.rotate( Math.PI * rotate/180, height, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    if ( (dd.dynType & GeDyn.mDynType_Rotate) != 0 && dd.rotate != 0) {
      g.rotate( Math.PI * dd.rotate/180, 

           (dd.x0 - getX())*original_width/width,
           (dd.y0 - getY()) * original_height / height);
    }
    AffineTransform save = g.getTransform();
    AffineTransform save_tmp;
    int rounds = 1;
    if ( fillLevel != 1F)
      rounds = 2;
    int oldColor = 0;
    for ( int i = 0; i < rounds; i++) {
      if ( rounds == 2) {
        switch ( i) {
        case 0:
	  if ( levelColorTone != GeColor.COLOR_TONE_NO) {
	    oldColor = colorTone;
	    colorTone = levelColorTone;
	  }
	  else if ( levelFillColor != GeColor.COLOR_NO) {
	    oldColor = fillColor;
	    fillColor = levelFillColor;
	  }
	  break;
        case 1:
	  if ( levelColorTone != GeColor.COLOR_TONE_NO)
	    colorTone = oldColor;
	  else if ( levelFillColor != GeColor.COLOR_NO)
	    fillColor = oldColor;
	  break;
        }
        switch ( levelDirection) {
        case Ge.DIRECTION_UP:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,fillLevel*original_height+Ge.cJBean_Offset,original_width,original_height));
	  else
	    g.setClip(new Rectangle2D.Float(0F,0F,original_width,fillLevel * original_height+Ge.cJBean_Offset));
	  break;
	case Ge.DIRECTION_DOWN:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,0F,original_width,(1-fillLevel)*original_height+Ge.cJBean_Offset));
	  else
	    g.setClip(new Rectangle2D.Float(0F,(1-fillLevel)*original_height+Ge.cJBean_Offset,original_width,original_height));
	  break;
	case Ge.DIRECTION_RIGHT:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(fillLevel*original_width+Ge.cJBean_Offset,0F,original_width,original_height));
	  else
	    g.setClip(new Rectangle2D.Float(0F,0F,fillLevel*width+Ge.cJBean_Offset,height));
	  break;
	case Ge.DIRECTION_LEFT:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,0F,(1-fillLevel)*original_width+Ge.cJBean_Offset,original_height));
	  else
	    g.setClip(new Rectangle2D.Float((1-fillLevel)*original_width+Ge.cJBean_Offset,0F,original_width,original_height));
	  break;
	}
      }
    {
    int fcolor = GeColor.getDrawtype(34, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    g.setColor(GeColor.getColor( fcolor));
    g.fill( shapes[0]);
    g.setColor(GeColor.shiftColor( fcolor, -2, colorInverse));
    g.fill( shapes[1]);
    g.setColor(GeColor.shiftColor( fcolor, 2, colorInverse));
    g.fill( shapes[2]);
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[0]);
    }
    g.setStroke( new BasicStroke(1F, BasicStroke.CAP_BUTT, BasicStroke.JOIN_ROUND));
    g.setColor(GeColor.getColor(39, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[3]);
    g.setStroke( new BasicStroke(1F, BasicStroke.CAP_BUTT, BasicStroke.JOIN_ROUND));
    g.setColor(GeColor.getColor(39, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[4]);
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(39, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[5]);
    }
    if ( rounds == 2)
      g.setClip(null);
    g.setTransform(save);
  }
  public Dimension getPreferredSize() { return size;}
  public Dimension getMinimumSize() { return size;}
}
protected class pwr_mbphoto extends GeComponent {
  // Dimension size;
  public pwr_mbphoto( JopSession session)
  {
    super( session);
    size = new Dimension( 20, 20);
  }
  public int original_width = 20;
  public int original_height = 20;
  Shape[] shapes = new Shape[] { 
    new Rectangle2D.Float(2F, 2F, 16F, 16F),
    new Polygon( new int[] { 2, 18, 16, 4, 4, 2}, new int[] { 2, 2, 4, 4, 16, 18}, 6), 
    new Polygon( new int[] { 18, 18, 16, 16, 4, 2}, new int[] { 18, 2, 4, 16, 16, 18}, 6), 
    new Rectangle2D.Float(6F, 6F, 8F, 8F),
    new Polygon( new int[] { 6, 14, 14, 6, 6, 6}, new int[] { 6, 6, 6, 6, 14, 14}, 6), 
    new Polygon( new int[] { 14, 14, 14, 14, 6, 6}, new int[] { 14, 6, 6, 14, 14, 14}, 6), 
    new Arc2D.Float(9F, 8F, 3F, 3F, 0F, 360F, Arc2D.PIE),
    new Arc2D.Float(9F, 8F, 3F, 3F, 35F, 140F, Arc2D.PIE),
    new Arc2D.Float(9F, 8F, 3F, 3F, 215F, 140F, Arc2D.PIE),
    new Arc2D.Float(9F, 8F, 3F, 3F, -5F, 40F, Arc2D.PIE),
    new Arc2D.Float(9F, 8F, 3F, 3F, 175F, 40F, Arc2D.PIE),
    new Arc2D.Float(9.15F, 8.15F, 2.7F, 2.7F, 0F, 360F, Arc2D.PIE),
    new Arc2D.Float(9F, 8F, 3F, 3F, 0F, 360F, Arc2D.PIE),
    new Polygon( new int[] { 9, 9, 14, 14, 9}, new int[] {14, 10, 10, 14, 14}, 5),
  };
  public void paintComponent(Graphics g1) {
    animationCount = 1;
    if ( !visible)
      return;
    Graphics2D g = (Graphics2D) g1;
    float width = getWidth();
    float height = getHeight();
    g.setRenderingHint(RenderingHints.KEY_ANTIALIASING,RenderingHints.VALUE_ANTIALIAS_ON);
    if ( 45.0 <= rotate && rotate < 135.0) {
      g.translate( width, 0.0); 
      g.rotate( Math.PI * rotate/180, 0.0, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else if ( 135.0 <= rotate && rotate < 225.0)
    {
      g.rotate( Math.PI * rotate/180, width/2, height/2);
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    }
    else if ( 225.0 <= rotate && rotate < 315.0)
    {
      g.translate( -height, 0.0);
      g.rotate( Math.PI * rotate/180, height, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    if ( (dd.dynType & GeDyn.mDynType_Rotate) != 0 && dd.rotate != 0) {
      g.rotate( Math.PI * dd.rotate/180, 

           (dd.x0 - getX())*original_width/width,
           (dd.y0 - getY()) * original_height / height);
    }
    AffineTransform save = g.getTransform();
    AffineTransform save_tmp;
    int rounds = 1;
    if ( fillLevel != 1F)
      rounds = 2;
    int oldColor = 0;
    for ( int i = 0; i < rounds; i++) {
      if ( rounds == 2) {
        switch ( i) {
        case 0:
	  if ( levelColorTone != GeColor.COLOR_TONE_NO) {
	    oldColor = colorTone;
	    colorTone = levelColorTone;
	  }
	  else if ( levelFillColor != GeColor.COLOR_NO) {
	    oldColor = fillColor;
	    fillColor = levelFillColor;
	  }
	  break;
        case 1:
	  if ( levelColorTone != GeColor.COLOR_TONE_NO)
	    colorTone = oldColor;
	  else if ( levelFillColor != GeColor.COLOR_NO)
	    fillColor = oldColor;
	  break;
        }
        switch ( levelDirection) {
        case Ge.DIRECTION_UP:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,fillLevel*original_height+Ge.cJBean_Offset,original_width,original_height));
	  else
	    g.setClip(new Rectangle2D.Float(0F,0F,original_width,fillLevel * original_height+Ge.cJBean_Offset));
	  break;
	case Ge.DIRECTION_DOWN:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,0F,original_width,(1-fillLevel)*original_height+Ge.cJBean_Offset));
	  else
	    g.setClip(new Rectangle2D.Float(0F,(1-fillLevel)*original_height+Ge.cJBean_Offset,original_width,original_height));
	  break;
	case Ge.DIRECTION_RIGHT:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(fillLevel*original_width+Ge.cJBean_Offset,0F,original_width,original_height));
	  else
	    g.setClip(new Rectangle2D.Float(0F,0F,fillLevel*width+Ge.cJBean_Offset,height));
	  break;
	case Ge.DIRECTION_LEFT:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,0F,(1-fillLevel)*original_width+Ge.cJBean_Offset,original_height));
	  else
	    g.setClip(new Rectangle2D.Float((1-fillLevel)*original_width+Ge.cJBean_Offset,0F,original_width,original_height));
	  break;
	}
      }
    {
    int fcolor = GeColor.getDrawtype(34, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    g.setColor(GeColor.getColor( fcolor));
    g.fill( shapes[0]);
    g.setColor(GeColor.shiftColor( fcolor, -2, colorInverse));
    g.fill( shapes[1]);
    g.setColor(GeColor.shiftColor( fcolor, 2, colorInverse));
    g.fill( shapes[2]);
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[0]);
    }
    {
    int fcolor = GeColor.getDrawtype(2, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    if ( shadow != 0) {
    g.setColor(GeColor.shiftColor( fcolor, -2, colorInverse));
    g.fill( shapes[4]);
    g.setColor(GeColor.shiftColor( fcolor, 2, colorInverse));
    g.fill( shapes[5]);
    }
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(39, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[3]);
    }
    if ( shadow == 0) {
    g.setColor(GeColor.getColor(39, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes[6]);
    } else {
    {
    int fcolor = GeColor.getDrawtype(39, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    g.setColor(GeColor.shiftColor( fcolor, -2, colorInverse));
    g.fill( shapes[7]);
    g.setColor(GeColor.shiftColor( fcolor, 2, colorInverse));
    g.fill( shapes[8]);
    g.setColor(GeColor.getColor( fcolor));
    g.fill( shapes[9]);
    g.fill( shapes[10]);
    g.fill( shapes[11]);
    }
    }
    g.setColor(GeColor.getColor(39, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes[13]);
    }
    if ( rounds == 2)
      g.setClip(null);
    g.setTransform(save);
  }
  public Dimension getPreferredSize() { return size;}
  public Dimension getMinimumSize() { return size;}
}
protected class pwr_mbdatasheet extends GeComponent {
  // Dimension size;
  public pwr_mbdatasheet( JopSession session)
  {
    super( session);
    size = new Dimension( 20, 20);
  }
  public int original_width = 20;
  public int original_height = 20;
  Shape[] shapes = new Shape[] { 
    new Rectangle2D.Float(2F, 2F, 16F, 16F),
    new Polygon( new int[] { 2, 18, 16, 4, 4, 2}, new int[] { 2, 2, 4, 4, 16, 18}, 6), 
    new Polygon( new int[] { 18, 18, 16, 16, 4, 2}, new int[] { 18, 2, 4, 16, 16, 18}, 6), 
    new Rectangle2D.Float(7F, 6F, 6F, 8F),
    new Polygon( new int[] { 7, 13, 13, 7, 7, 7}, new int[] { 6, 6, 6, 6, 14, 14}, 6), 
    new Polygon( new int[] { 13, 13, 13, 13, 7, 7}, new int[] { 14, 6, 6, 14, 14, 14}, 6), 
    new Polygon( new int[] { 7, 9, 7, 7}, new int[] {10, 6, 6, 10}, 4),
  };
  public void paintComponent(Graphics g1) {
    animationCount = 1;
    if ( !visible)
      return;
    Graphics2D g = (Graphics2D) g1;
    float width = getWidth();
    float height = getHeight();
    g.setRenderingHint(RenderingHints.KEY_ANTIALIASING,RenderingHints.VALUE_ANTIALIAS_ON);
    if ( 45.0 <= rotate && rotate < 135.0) {
      g.translate( width, 0.0); 
      g.rotate( Math.PI * rotate/180, 0.0, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else if ( 135.0 <= rotate && rotate < 225.0)
    {
      g.rotate( Math.PI * rotate/180, width/2, height/2);
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    }
    else if ( 225.0 <= rotate && rotate < 315.0)
    {
      g.translate( -height, 0.0);
      g.rotate( Math.PI * rotate/180, height, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    if ( (dd.dynType & GeDyn.mDynType_Rotate) != 0 && dd.rotate != 0) {
      g.rotate( Math.PI * dd.rotate/180, 

           (dd.x0 - getX())*original_width/width,
           (dd.y0 - getY()) * original_height / height);
    }
    AffineTransform save = g.getTransform();
    AffineTransform save_tmp;
    int rounds = 1;
    if ( fillLevel != 1F)
      rounds = 2;
    int oldColor = 0;
    for ( int i = 0; i < rounds; i++) {
      if ( rounds == 2) {
        switch ( i) {
        case 0:
	  if ( levelColorTone != GeColor.COLOR_TONE_NO) {
	    oldColor = colorTone;
	    colorTone = levelColorTone;
	  }
	  else if ( levelFillColor != GeColor.COLOR_NO) {
	    oldColor = fillColor;
	    fillColor = levelFillColor;
	  }
	  break;
        case 1:
	  if ( levelColorTone != GeColor.COLOR_TONE_NO)
	    colorTone = oldColor;
	  else if ( levelFillColor != GeColor.COLOR_NO)
	    fillColor = oldColor;
	  break;
        }
        switch ( levelDirection) {
        case Ge.DIRECTION_UP:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,fillLevel*original_height+Ge.cJBean_Offset,original_width,original_height));
	  else
	    g.setClip(new Rectangle2D.Float(0F,0F,original_width,fillLevel * original_height+Ge.cJBean_Offset));
	  break;
	case Ge.DIRECTION_DOWN:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,0F,original_width,(1-fillLevel)*original_height+Ge.cJBean_Offset));
	  else
	    g.setClip(new Rectangle2D.Float(0F,(1-fillLevel)*original_height+Ge.cJBean_Offset,original_width,original_height));
	  break;
	case Ge.DIRECTION_RIGHT:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(fillLevel*original_width+Ge.cJBean_Offset,0F,original_width,original_height));
	  else
	    g.setClip(new Rectangle2D.Float(0F,0F,fillLevel*width+Ge.cJBean_Offset,height));
	  break;
	case Ge.DIRECTION_LEFT:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,0F,(1-fillLevel)*original_width+Ge.cJBean_Offset,original_height));
	  else
	    g.setClip(new Rectangle2D.Float((1-fillLevel)*original_width+Ge.cJBean_Offset,0F,original_width,original_height));
	  break;
	}
      }
    {
    int fcolor = GeColor.getDrawtype(34, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    g.setColor(GeColor.getColor( fcolor));
    g.fill( shapes[0]);
    g.setColor(GeColor.shiftColor( fcolor, -2, colorInverse));
    g.fill( shapes[1]);
    g.setColor(GeColor.shiftColor( fcolor, 2, colorInverse));
    g.fill( shapes[2]);
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[0]);
    }
    {
    int fcolor = GeColor.getDrawtype(39, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    if ( shadow != 0) {
    g.setColor(GeColor.shiftColor( fcolor, -2, colorInverse));
    g.fill( shapes[4]);
    g.setColor(GeColor.shiftColor( fcolor, 2, colorInverse));
    g.fill( shapes[5]);
    }
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(39, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[3]);
    }
    g.setColor(GeColor.getColor(39, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes[6]);
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(39, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[6]);
    }
    if ( rounds == 2)
      g.setClip(null);
    g.setTransform(save);
  }
  public Dimension getPreferredSize() { return size;}
  public Dimension getMinimumSize() { return size;}
}
protected class pwr_mbopenplc extends GeComponent {
  // Dimension size;
  public pwr_mbopenplc( JopSession session)
  {
    super( session);
    size = new Dimension( 20, 20);
  }
  public int original_width = 20;
  public int original_height = 20;
  Shape[] shapes = new Shape[] { 
    new Rectangle2D.Float(2F, 2F, 16F, 16F),
    new Polygon( new int[] { 2, 18, 16, 4, 4, 2}, new int[] { 2, 2, 4, 4, 16, 18}, 6), 
    new Polygon( new int[] { 18, 18, 16, 16, 4, 2}, new int[] { 18, 2, 4, 16, 16, 18}, 6), 
    new Rectangle2D.Float(8F, 6F, 4F, 8F),
    new Polygon( new int[] { 8, 12, 12, 8, 8, 8}, new int[] { 6, 6, 6, 6, 14, 14}, 6), 
    new Polygon( new int[] { 12, 12, 12, 12, 8, 8}, new int[] { 14, 6, 6, 14, 14, 14}, 6), 
    new Line2D.Float( 8F, 8F, 6F, 8F),
    new Line2D.Float( 8F, 12F, 6F, 12F),
    new Line2D.Float( 14F, 8F, 12F, 8F),
  };
  public void paintComponent(Graphics g1) {
    animationCount = 1;
    if ( !visible)
      return;
    Graphics2D g = (Graphics2D) g1;
    float width = getWidth();
    float height = getHeight();
    g.setRenderingHint(RenderingHints.KEY_ANTIALIASING,RenderingHints.VALUE_ANTIALIAS_ON);
    if ( 45.0 <= rotate && rotate < 135.0) {
      g.translate( width, 0.0); 
      g.rotate( Math.PI * rotate/180, 0.0, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else if ( 135.0 <= rotate && rotate < 225.0)
    {
      g.rotate( Math.PI * rotate/180, width/2, height/2);
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    }
    else if ( 225.0 <= rotate && rotate < 315.0)
    {
      g.translate( -height, 0.0);
      g.rotate( Math.PI * rotate/180, height, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    if ( (dd.dynType & GeDyn.mDynType_Rotate) != 0 && dd.rotate != 0) {
      g.rotate( Math.PI * dd.rotate/180, 

           (dd.x0 - getX())*original_width/width,
           (dd.y0 - getY()) * original_height / height);
    }
    AffineTransform save = g.getTransform();
    AffineTransform save_tmp;
    int rounds = 1;
    if ( fillLevel != 1F)
      rounds = 2;
    int oldColor = 0;
    for ( int i = 0; i < rounds; i++) {
      if ( rounds == 2) {
        switch ( i) {
        case 0:
	  if ( levelColorTone != GeColor.COLOR_TONE_NO) {
	    oldColor = colorTone;
	    colorTone = levelColorTone;
	  }
	  else if ( levelFillColor != GeColor.COLOR_NO) {
	    oldColor = fillColor;
	    fillColor = levelFillColor;
	  }
	  break;
        case 1:
	  if ( levelColorTone != GeColor.COLOR_TONE_NO)
	    colorTone = oldColor;
	  else if ( levelFillColor != GeColor.COLOR_NO)
	    fillColor = oldColor;
	  break;
        }
        switch ( levelDirection) {
        case Ge.DIRECTION_UP:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,fillLevel*original_height+Ge.cJBean_Offset,original_width,original_height));
	  else
	    g.setClip(new Rectangle2D.Float(0F,0F,original_width,fillLevel * original_height+Ge.cJBean_Offset));
	  break;
	case Ge.DIRECTION_DOWN:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,0F,original_width,(1-fillLevel)*original_height+Ge.cJBean_Offset));
	  else
	    g.setClip(new Rectangle2D.Float(0F,(1-fillLevel)*original_height+Ge.cJBean_Offset,original_width,original_height));
	  break;
	case Ge.DIRECTION_RIGHT:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(fillLevel*original_width+Ge.cJBean_Offset,0F,original_width,original_height));
	  else
	    g.setClip(new Rectangle2D.Float(0F,0F,fillLevel*width+Ge.cJBean_Offset,height));
	  break;
	case Ge.DIRECTION_LEFT:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,0F,(1-fillLevel)*original_width+Ge.cJBean_Offset,original_height));
	  else
	    g.setClip(new Rectangle2D.Float((1-fillLevel)*original_width+Ge.cJBean_Offset,0F,original_width,original_height));
	  break;
	}
      }
    {
    int fcolor = GeColor.getDrawtype(34, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    g.setColor(GeColor.getColor( fcolor));
    g.fill( shapes[0]);
    g.setColor(GeColor.shiftColor( fcolor, -2, colorInverse));
    g.fill( shapes[1]);
    g.setColor(GeColor.shiftColor( fcolor, 2, colorInverse));
    g.fill( shapes[2]);
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[0]);
    }
    {
    int fcolor = GeColor.getDrawtype(39, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    if ( shadow != 0) {
    g.setColor(GeColor.shiftColor( fcolor, -2, colorInverse));
    g.fill( shapes[4]);
    g.setColor(GeColor.shiftColor( fcolor, 2, colorInverse));
    g.fill( shapes[5]);
    }
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(39, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[3]);
    }
    g.setStroke( new BasicStroke(1F, BasicStroke.CAP_BUTT, BasicStroke.JOIN_ROUND));
    g.setColor(GeColor.getColor(39, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[6]);
    g.setStroke( new BasicStroke(1F, BasicStroke.CAP_BUTT, BasicStroke.JOIN_ROUND));
    g.setColor(GeColor.getColor(39, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[7]);
    g.setStroke( new BasicStroke(1F, BasicStroke.CAP_BUTT, BasicStroke.JOIN_ROUND));
    g.setColor(GeColor.getColor(39, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[8]);
    }
    if ( rounds == 2)
      g.setClip(null);
    g.setTransform(save);
  }
  public Dimension getPreferredSize() { return size;}
  public Dimension getMinimumSize() { return size;}
}
protected class pwr_mbcircuitdiagram extends GeComponent {
  // Dimension size;
  public pwr_mbcircuitdiagram( JopSession session)
  {
    super( session);
    size = new Dimension( 20, 20);
  }
  public int original_width = 20;
  public int original_height = 20;
  Shape[] shapes = new Shape[] { 
    new Rectangle2D.Float(2F, 2F, 16F, 16F),
    new Polygon( new int[] { 2, 18, 16, 4, 4, 2}, new int[] { 2, 2, 4, 4, 16, 18}, 6), 
    new Polygon( new int[] { 18, 18, 16, 16, 4, 2}, new int[] { 18, 2, 4, 16, 16, 18}, 6), 
    new Rectangle2D.Float(5F, 7F, 10F, 7F),
    new Polygon( new int[] { 5, 15, 15, 5, 5, 5}, new int[] { 7, 7, 7, 7, 14, 14}, 6), 
    new Polygon( new int[] { 15, 15, 15, 15, 5, 5}, new int[] { 14, 7, 7, 14, 14, 14}, 6), 
    new Line2D.Float( 15F, 13F, 5F, 13F),
    new Line2D.Float( 11F, 9F, 5F, 9F),
    new Line2D.Float( 11F, 11F, 11F, 9F),
    new Rectangle2D.Float(7F, 11F, 4F, 1F),
    new Polygon( new int[] { 7, 11, 11, 7, 7, 7}, new int[] { 11, 11, 11, 11, 12, 12}, 6), 
    new Polygon( new int[] { 11, 11, 11, 11, 7, 7}, new int[] { 12, 11, 11, 12, 12, 12}, 6), 
    new Line2D.Float( 15F, 10F, 11F, 10F),
    new Line2D.Float( 15F, 8F, 11F, 8F),
  };
  public void paintComponent(Graphics g1) {
    animationCount = 1;
    if ( !visible)
      return;
    Graphics2D g = (Graphics2D) g1;
    float width = getWidth();
    float height = getHeight();
    g.setRenderingHint(RenderingHints.KEY_ANTIALIASING,RenderingHints.VALUE_ANTIALIAS_ON);
    if ( 45.0 <= rotate && rotate < 135.0) {
      g.translate( width, 0.0); 
      g.rotate( Math.PI * rotate/180, 0.0, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else if ( 135.0 <= rotate && rotate < 225.0)
    {
      g.rotate( Math.PI * rotate/180, width/2, height/2);
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    }
    else if ( 225.0 <= rotate && rotate < 315.0)
    {
      g.translate( -height, 0.0);
      g.rotate( Math.PI * rotate/180, height, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    if ( (dd.dynType & GeDyn.mDynType_Rotate) != 0 && dd.rotate != 0) {
      g.rotate( Math.PI * dd.rotate/180, 

           (dd.x0 - getX())*original_width/width,
           (dd.y0 - getY()) * original_height / height);
    }
    AffineTransform save = g.getTransform();
    AffineTransform save_tmp;
    int rounds = 1;
    if ( fillLevel != 1F)
      rounds = 2;
    int oldColor = 0;
    for ( int i = 0; i < rounds; i++) {
      if ( rounds == 2) {
        switch ( i) {
        case 0:
	  if ( levelColorTone != GeColor.COLOR_TONE_NO) {
	    oldColor = colorTone;
	    colorTone = levelColorTone;
	  }
	  else if ( levelFillColor != GeColor.COLOR_NO) {
	    oldColor = fillColor;
	    fillColor = levelFillColor;
	  }
	  break;
        case 1:
	  if ( levelColorTone != GeColor.COLOR_TONE_NO)
	    colorTone = oldColor;
	  else if ( levelFillColor != GeColor.COLOR_NO)
	    fillColor = oldColor;
	  break;
        }
        switch ( levelDirection) {
        case Ge.DIRECTION_UP:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,fillLevel*original_height+Ge.cJBean_Offset,original_width,original_height));
	  else
	    g.setClip(new Rectangle2D.Float(0F,0F,original_width,fillLevel * original_height+Ge.cJBean_Offset));
	  break;
	case Ge.DIRECTION_DOWN:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,0F,original_width,(1-fillLevel)*original_height+Ge.cJBean_Offset));
	  else
	    g.setClip(new Rectangle2D.Float(0F,(1-fillLevel)*original_height+Ge.cJBean_Offset,original_width,original_height));
	  break;
	case Ge.DIRECTION_RIGHT:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(fillLevel*original_width+Ge.cJBean_Offset,0F,original_width,original_height));
	  else
	    g.setClip(new Rectangle2D.Float(0F,0F,fillLevel*width+Ge.cJBean_Offset,height));
	  break;
	case Ge.DIRECTION_LEFT:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,0F,(1-fillLevel)*original_width+Ge.cJBean_Offset,original_height));
	  else
	    g.setClip(new Rectangle2D.Float((1-fillLevel)*original_width+Ge.cJBean_Offset,0F,original_width,original_height));
	  break;
	}
      }
    {
    int fcolor = GeColor.getDrawtype(34, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    g.setColor(GeColor.getColor( fcolor));
    g.fill( shapes[0]);
    g.setColor(GeColor.shiftColor( fcolor, -2, colorInverse));
    g.fill( shapes[1]);
    g.setColor(GeColor.shiftColor( fcolor, 2, colorInverse));
    g.fill( shapes[2]);
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[0]);
    }
    {
    int fcolor = GeColor.getDrawtype(39, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    if ( shadow != 0) {
    g.setColor(GeColor.shiftColor( fcolor, -2, colorInverse));
    g.fill( shapes[4]);
    g.setColor(GeColor.shiftColor( fcolor, 2, colorInverse));
    g.fill( shapes[5]);
    }
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(39, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[3]);
    }
    g.setStroke( new BasicStroke(1F, BasicStroke.CAP_BUTT, BasicStroke.JOIN_ROUND));
    g.setColor(GeColor.getColor(39, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[6]);
    g.setStroke( new BasicStroke(1F, BasicStroke.CAP_BUTT, BasicStroke.JOIN_ROUND));
    g.setColor(GeColor.getColor(39, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[7]);
    g.setStroke( new BasicStroke(1F, BasicStroke.CAP_BUTT, BasicStroke.JOIN_ROUND));
    g.setColor(GeColor.getColor(39, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[8]);
    {
    int fcolor = GeColor.getDrawtype(39, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    g.setColor(GeColor.getColor( fcolor));
    g.fill( shapes[9]);
    if ( shadow != 0) {
    g.setColor(GeColor.shiftColor( fcolor, -2, colorInverse));
    g.fill( shapes[10]);
    g.setColor(GeColor.shiftColor( fcolor, 2, colorInverse));
    g.fill( shapes[11]);
    }
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(39, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[9]);
    }
    g.setStroke( new BasicStroke(1F, BasicStroke.CAP_BUTT, BasicStroke.JOIN_ROUND));
    g.setColor(GeColor.getColor(39, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[12]);
    g.setStroke( new BasicStroke(1F, BasicStroke.CAP_BUTT, BasicStroke.JOIN_ROUND));
    g.setColor(GeColor.getColor(39, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[13]);
    }
    if ( rounds == 2)
      g.setClip(null);
    g.setTransform(save);
  }
  public Dimension getPreferredSize() { return size;}
  public Dimension getMinimumSize() { return size;}
}
protected class pwr_mbrtnavigator extends GeComponent {
  // Dimension size;
  public pwr_mbrtnavigator( JopSession session)
  {
    super( session);
    size = new Dimension( 20, 20);
  }
  public int original_width = 20;
  public int original_height = 20;
  Shape[] shapes = new Shape[] { 
    new Rectangle2D.Float(2F, 2F, 16F, 16F),
    new Polygon( new int[] { 2, 18, 16, 4, 4, 2}, new int[] { 2, 2, 4, 4, 16, 18}, 6), 
    new Polygon( new int[] { 18, 18, 16, 16, 4, 2}, new int[] { 18, 2, 4, 16, 16, 18}, 6), 
    new Rectangle2D.Float(6F, 6F, 4F, 3F),
    new Polygon( new int[] { 6, 10, 10, 6, 6, 6}, new int[] { 6, 6, 6, 6, 9, 9}, 6), 
    new Polygon( new int[] { 10, 10, 10, 10, 6, 6}, new int[] { 9, 6, 6, 9, 9, 9}, 6), 
    new Rectangle2D.Float(8F, 10F, 4F, 5F),
    new Polygon( new int[] { 8, 12, 12, 8, 8, 8}, new int[] { 10, 10, 10, 10, 15, 15}, 6), 
    new Polygon( new int[] { 12, 12, 12, 12, 8, 8}, new int[] { 15, 10, 10, 15, 15, 15}, 6), 
  };
  public void paintComponent(Graphics g1) {
    animationCount = 1;
    if ( !visible)
      return;
    Graphics2D g = (Graphics2D) g1;
    float width = getWidth();
    float height = getHeight();
    g.setRenderingHint(RenderingHints.KEY_ANTIALIASING,RenderingHints.VALUE_ANTIALIAS_ON);
    if ( 45.0 <= rotate && rotate < 135.0) {
      g.translate( width, 0.0); 
      g.rotate( Math.PI * rotate/180, 0.0, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else if ( 135.0 <= rotate && rotate < 225.0)
    {
      g.rotate( Math.PI * rotate/180, width/2, height/2);
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    }
    else if ( 225.0 <= rotate && rotate < 315.0)
    {
      g.translate( -height, 0.0);
      g.rotate( Math.PI * rotate/180, height, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    if ( (dd.dynType & GeDyn.mDynType_Rotate) != 0 && dd.rotate != 0) {
      g.rotate( Math.PI * dd.rotate/180, 

           (dd.x0 - getX())*original_width/width,
           (dd.y0 - getY()) * original_height / height);
    }
    AffineTransform save = g.getTransform();
    AffineTransform save_tmp;
    int rounds = 1;
    if ( fillLevel != 1F)
      rounds = 2;
    int oldColor = 0;
    for ( int i = 0; i < rounds; i++) {
      if ( rounds == 2) {
        switch ( i) {
        case 0:
	  if ( levelColorTone != GeColor.COLOR_TONE_NO) {
	    oldColor = colorTone;
	    colorTone = levelColorTone;
	  }
	  else if ( levelFillColor != GeColor.COLOR_NO) {
	    oldColor = fillColor;
	    fillColor = levelFillColor;
	  }
	  break;
        case 1:
	  if ( levelColorTone != GeColor.COLOR_TONE_NO)
	    colorTone = oldColor;
	  else if ( levelFillColor != GeColor.COLOR_NO)
	    fillColor = oldColor;
	  break;
        }
        switch ( levelDirection) {
        case Ge.DIRECTION_UP:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,fillLevel*original_height+Ge.cJBean_Offset,original_width,original_height));
	  else
	    g.setClip(new Rectangle2D.Float(0F,0F,original_width,fillLevel * original_height+Ge.cJBean_Offset));
	  break;
	case Ge.DIRECTION_DOWN:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,0F,original_width,(1-fillLevel)*original_height+Ge.cJBean_Offset));
	  else
	    g.setClip(new Rectangle2D.Float(0F,(1-fillLevel)*original_height+Ge.cJBean_Offset,original_width,original_height));
	  break;
	case Ge.DIRECTION_RIGHT:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(fillLevel*original_width+Ge.cJBean_Offset,0F,original_width,original_height));
	  else
	    g.setClip(new Rectangle2D.Float(0F,0F,fillLevel*width+Ge.cJBean_Offset,height));
	  break;
	case Ge.DIRECTION_LEFT:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,0F,(1-fillLevel)*original_width+Ge.cJBean_Offset,original_height));
	  else
	    g.setClip(new Rectangle2D.Float((1-fillLevel)*original_width+Ge.cJBean_Offset,0F,original_width,original_height));
	  break;
	}
      }
    {
    int fcolor = GeColor.getDrawtype(34, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    g.setColor(GeColor.getColor( fcolor));
    g.fill( shapes[0]);
    g.setColor(GeColor.shiftColor( fcolor, -2, colorInverse));
    g.fill( shapes[1]);
    g.setColor(GeColor.shiftColor( fcolor, 2, colorInverse));
    g.fill( shapes[2]);
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[0]);
    }
    {
    int fcolor = GeColor.getDrawtype(39, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    g.setColor(GeColor.getColor( fcolor));
    g.fill( shapes[3]);
    if ( shadow != 0) {
    g.setColor(GeColor.shiftColor( fcolor, -2, colorInverse));
    g.fill( shapes[4]);
    g.setColor(GeColor.shiftColor( fcolor, 2, colorInverse));
    g.fill( shapes[5]);
    }
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(39, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[3]);
    }
    {
    int fcolor = GeColor.getDrawtype(39, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    g.setColor(GeColor.getColor( fcolor));
    g.fill( shapes[6]);
    if ( shadow != 0) {
    g.setColor(GeColor.shiftColor( fcolor, -2, colorInverse));
    g.fill( shapes[7]);
    g.setColor(GeColor.shiftColor( fcolor, 2, colorInverse));
    g.fill( shapes[8]);
    }
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(39, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[6]);
    }
    }
    if ( rounds == 2)
      g.setClip(null);
    g.setTransform(save);
  }
  public Dimension getPreferredSize() { return size;}
  public Dimension getMinimumSize() { return size;}
}
protected class pwr_mbhelpclass extends GeComponent {
  // Dimension size;
  public pwr_mbhelpclass( JopSession session)
  {
    super( session);
    size = new Dimension( 20, 20);
  }
  public int original_width = 20;
  public int original_height = 20;
  Shape[] shapes = new Shape[] { 
    new Rectangle2D.Float(2F, 2F, 16F, 16F),
    new Polygon( new int[] { 2, 18, 16, 4, 4, 2}, new int[] { 2, 2, 4, 4, 16, 18}, 6), 
    new Polygon( new int[] { 18, 18, 16, 16, 4, 2}, new int[] { 18, 2, 4, 16, 16, 18}, 6), 
    new Polygon( new int[] { 8, 8, 12, 12, 10, 10, 10, 12, 12, 8}, new int[] {8, 7, 7, 10, 10, 12, 10, 10, 7, 7}, 10),
    new Line2D.Float( 10F, 13.772F, 10F, 13F),
  };
  public void paintComponent(Graphics g1) {
    animationCount = 1;
    if ( !visible)
      return;
    Graphics2D g = (Graphics2D) g1;
    float width = getWidth();
    float height = getHeight();
    g.setRenderingHint(RenderingHints.KEY_ANTIALIASING,RenderingHints.VALUE_ANTIALIAS_ON);
    if ( 45.0 <= rotate && rotate < 135.0) {
      g.translate( width, 0.0); 
      g.rotate( Math.PI * rotate/180, 0.0, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else if ( 135.0 <= rotate && rotate < 225.0)
    {
      g.rotate( Math.PI * rotate/180, width/2, height/2);
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    }
    else if ( 225.0 <= rotate && rotate < 315.0)
    {
      g.translate( -height, 0.0);
      g.rotate( Math.PI * rotate/180, height, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    if ( (dd.dynType & GeDyn.mDynType_Rotate) != 0 && dd.rotate != 0) {
      g.rotate( Math.PI * dd.rotate/180, 

           (dd.x0 - getX())*original_width/width,
           (dd.y0 - getY()) * original_height / height);
    }
    AffineTransform save = g.getTransform();
    AffineTransform save_tmp;
    int rounds = 1;
    if ( fillLevel != 1F)
      rounds = 2;
    int oldColor = 0;
    for ( int i = 0; i < rounds; i++) {
      if ( rounds == 2) {
        switch ( i) {
        case 0:
	  if ( levelColorTone != GeColor.COLOR_TONE_NO) {
	    oldColor = colorTone;
	    colorTone = levelColorTone;
	  }
	  else if ( levelFillColor != GeColor.COLOR_NO) {
	    oldColor = fillColor;
	    fillColor = levelFillColor;
	  }
	  break;
        case 1:
	  if ( levelColorTone != GeColor.COLOR_TONE_NO)
	    colorTone = oldColor;
	  else if ( levelFillColor != GeColor.COLOR_NO)
	    fillColor = oldColor;
	  break;
        }
        switch ( levelDirection) {
        case Ge.DIRECTION_UP:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,fillLevel*original_height+Ge.cJBean_Offset,original_width,original_height));
	  else
	    g.setClip(new Rectangle2D.Float(0F,0F,original_width,fillLevel * original_height+Ge.cJBean_Offset));
	  break;
	case Ge.DIRECTION_DOWN:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,0F,original_width,(1-fillLevel)*original_height+Ge.cJBean_Offset));
	  else
	    g.setClip(new Rectangle2D.Float(0F,(1-fillLevel)*original_height+Ge.cJBean_Offset,original_width,original_height));
	  break;
	case Ge.DIRECTION_RIGHT:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(fillLevel*original_width+Ge.cJBean_Offset,0F,original_width,original_height));
	  else
	    g.setClip(new Rectangle2D.Float(0F,0F,fillLevel*width+Ge.cJBean_Offset,height));
	  break;
	case Ge.DIRECTION_LEFT:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,0F,(1-fillLevel)*original_width+Ge.cJBean_Offset,original_height));
	  else
	    g.setClip(new Rectangle2D.Float((1-fillLevel)*original_width+Ge.cJBean_Offset,0F,original_width,original_height));
	  break;
	}
      }
    {
    int fcolor = GeColor.getDrawtype(34, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    g.setColor(GeColor.getColor( fcolor));
    g.fill( shapes[0]);
    g.setColor(GeColor.shiftColor( fcolor, -2, colorInverse));
    g.fill( shapes[1]);
    g.setColor(GeColor.shiftColor( fcolor, 2, colorInverse));
    g.fill( shapes[2]);
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[0]);
    }
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(39, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[3]);
    g.setStroke( new BasicStroke(1F, BasicStroke.CAP_BUTT, BasicStroke.JOIN_ROUND));
    g.setColor(GeColor.getColor(39, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[4]);
    }
    if ( rounds == 2)
      g.setClip(null);
    g.setTransform(save);
  }
  public Dimension getPreferredSize() { return size;}
  public Dimension getMinimumSize() { return size;}
}
protected class pwr_mbblockevents extends GeComponent {
  // Dimension size;
  public pwr_mbblockevents( JopSession session)
  {
    super( session);
    size = new Dimension( 20, 20);
  }
  public int original_width = 20;
  public int original_height = 20;
  Shape[] shapes = new Shape[] { 
    new Rectangle2D.Float(2F, 2F, 16F, 16F),
    new Polygon( new int[] { 2, 18, 16, 4, 4, 2}, new int[] { 2, 2, 4, 4, 16, 18}, 6), 
    new Polygon( new int[] { 18, 18, 16, 16, 4, 2}, new int[] { 18, 2, 4, 16, 16, 18}, 6), 
    new Line2D.Float( 10F, 13.772F, 10F, 13F),
    new Line2D.Float( 10F, 11F, 10F, 6F),
    new Line2D.Float( 14F, 14F, 6F, 6F),
    new Line2D.Float( 14F, 6F, 6F, 14F),
  };
  public void paintComponent(Graphics g1) {
    animationCount = 1;
    if ( !visible)
      return;
    Graphics2D g = (Graphics2D) g1;
    float width = getWidth();
    float height = getHeight();
    g.setRenderingHint(RenderingHints.KEY_ANTIALIASING,RenderingHints.VALUE_ANTIALIAS_ON);
    if ( 45.0 <= rotate && rotate < 135.0) {
      g.translate( width, 0.0); 
      g.rotate( Math.PI * rotate/180, 0.0, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else if ( 135.0 <= rotate && rotate < 225.0)
    {
      g.rotate( Math.PI * rotate/180, width/2, height/2);
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    }
    else if ( 225.0 <= rotate && rotate < 315.0)
    {
      g.translate( -height, 0.0);
      g.rotate( Math.PI * rotate/180, height, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    if ( (dd.dynType & GeDyn.mDynType_Rotate) != 0 && dd.rotate != 0) {
      g.rotate( Math.PI * dd.rotate/180, 

           (dd.x0 - getX())*original_width/width,
           (dd.y0 - getY()) * original_height / height);
    }
    AffineTransform save = g.getTransform();
    AffineTransform save_tmp;
    int rounds = 1;
    if ( fillLevel != 1F)
      rounds = 2;
    int oldColor = 0;
    for ( int i = 0; i < rounds; i++) {
      if ( rounds == 2) {
        switch ( i) {
        case 0:
	  if ( levelColorTone != GeColor.COLOR_TONE_NO) {
	    oldColor = colorTone;
	    colorTone = levelColorTone;
	  }
	  else if ( levelFillColor != GeColor.COLOR_NO) {
	    oldColor = fillColor;
	    fillColor = levelFillColor;
	  }
	  break;
        case 1:
	  if ( levelColorTone != GeColor.COLOR_TONE_NO)
	    colorTone = oldColor;
	  else if ( levelFillColor != GeColor.COLOR_NO)
	    fillColor = oldColor;
	  break;
        }
        switch ( levelDirection) {
        case Ge.DIRECTION_UP:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,fillLevel*original_height+Ge.cJBean_Offset,original_width,original_height));
	  else
	    g.setClip(new Rectangle2D.Float(0F,0F,original_width,fillLevel * original_height+Ge.cJBean_Offset));
	  break;
	case Ge.DIRECTION_DOWN:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,0F,original_width,(1-fillLevel)*original_height+Ge.cJBean_Offset));
	  else
	    g.setClip(new Rectangle2D.Float(0F,(1-fillLevel)*original_height+Ge.cJBean_Offset,original_width,original_height));
	  break;
	case Ge.DIRECTION_RIGHT:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(fillLevel*original_width+Ge.cJBean_Offset,0F,original_width,original_height));
	  else
	    g.setClip(new Rectangle2D.Float(0F,0F,fillLevel*width+Ge.cJBean_Offset,height));
	  break;
	case Ge.DIRECTION_LEFT:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,0F,(1-fillLevel)*original_width+Ge.cJBean_Offset,original_height));
	  else
	    g.setClip(new Rectangle2D.Float((1-fillLevel)*original_width+Ge.cJBean_Offset,0F,original_width,original_height));
	  break;
	}
      }
    {
    int fcolor = GeColor.getDrawtype(34, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    g.setColor(GeColor.getColor( fcolor));
    g.fill( shapes[0]);
    g.setColor(GeColor.shiftColor( fcolor, -2, colorInverse));
    g.fill( shapes[1]);
    g.setColor(GeColor.shiftColor( fcolor, 2, colorInverse));
    g.fill( shapes[2]);
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[0]);
    }
    g.setStroke( new BasicStroke(1F, BasicStroke.CAP_BUTT, BasicStroke.JOIN_ROUND));
    g.setColor(GeColor.getColor(39, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[3]);
    g.setStroke( new BasicStroke(1F, BasicStroke.CAP_BUTT, BasicStroke.JOIN_ROUND));
    g.setColor(GeColor.getColor(39, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[4]);
    g.setStroke( new BasicStroke(1F, BasicStroke.CAP_BUTT, BasicStroke.JOIN_ROUND));
    g.setColor(GeColor.getColor(39, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[5]);
    g.setStroke( new BasicStroke(1F, BasicStroke.CAP_BUTT, BasicStroke.JOIN_ROUND));
    g.setColor(GeColor.getColor(39, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[6]);
    }
    if ( rounds == 2)
      g.setClip(null);
    g.setTransform(save);
  }
  public Dimension getPreferredSize() { return size;}
  public Dimension getMinimumSize() { return size;}
}
protected class pwr_mbhistevent extends GeComponent {
  // Dimension size;
  public pwr_mbhistevent( JopSession session)
  {
    super( session);
    size = new Dimension( 20, 20);
  }
  public int original_width = 20;
  public int original_height = 20;
  Shape[] shapes = new Shape[] { 
    new Rectangle2D.Float(2F, 2F, 16F, 16F),
    new Polygon( new int[] { 2, 18, 16, 4, 4, 2}, new int[] { 2, 2, 4, 4, 16, 18}, 6), 
    new Polygon( new int[] { 18, 18, 16, 16, 4, 2}, new int[] { 18, 2, 4, 16, 16, 18}, 6), 
    new Line2D.Float( 10F, 13.772F, 10F, 13F),
    new Polygon( new int[] { 8, 6, 10, 14, 12, 14, 10, 6}, new int[] {14, 14, 6, 14, 14, 14, 6, 14}, 8),
    new Line2D.Float( 10F, 11F, 10F, 8F),
  };
  public void paintComponent(Graphics g1) {
    animationCount = 1;
    if ( !visible)
      return;
    Graphics2D g = (Graphics2D) g1;
    float width = getWidth();
    float height = getHeight();
    g.setRenderingHint(RenderingHints.KEY_ANTIALIASING,RenderingHints.VALUE_ANTIALIAS_ON);
    if ( 45.0 <= rotate && rotate < 135.0) {
      g.translate( width, 0.0); 
      g.rotate( Math.PI * rotate/180, 0.0, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else if ( 135.0 <= rotate && rotate < 225.0)
    {
      g.rotate( Math.PI * rotate/180, width/2, height/2);
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    }
    else if ( 225.0 <= rotate && rotate < 315.0)
    {
      g.translate( -height, 0.0);
      g.rotate( Math.PI * rotate/180, height, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    if ( (dd.dynType & GeDyn.mDynType_Rotate) != 0 && dd.rotate != 0) {
      g.rotate( Math.PI * dd.rotate/180, 

           (dd.x0 - getX())*original_width/width,
           (dd.y0 - getY()) * original_height / height);
    }
    AffineTransform save = g.getTransform();
    AffineTransform save_tmp;
    int rounds = 1;
    if ( fillLevel != 1F)
      rounds = 2;
    int oldColor = 0;
    for ( int i = 0; i < rounds; i++) {
      if ( rounds == 2) {
        switch ( i) {
        case 0:
	  if ( levelColorTone != GeColor.COLOR_TONE_NO) {
	    oldColor = colorTone;
	    colorTone = levelColorTone;
	  }
	  else if ( levelFillColor != GeColor.COLOR_NO) {
	    oldColor = fillColor;
	    fillColor = levelFillColor;
	  }
	  break;
        case 1:
	  if ( levelColorTone != GeColor.COLOR_TONE_NO)
	    colorTone = oldColor;
	  else if ( levelFillColor != GeColor.COLOR_NO)
	    fillColor = oldColor;
	  break;
        }
        switch ( levelDirection) {
        case Ge.DIRECTION_UP:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,fillLevel*original_height+Ge.cJBean_Offset,original_width,original_height));
	  else
	    g.setClip(new Rectangle2D.Float(0F,0F,original_width,fillLevel * original_height+Ge.cJBean_Offset));
	  break;
	case Ge.DIRECTION_DOWN:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,0F,original_width,(1-fillLevel)*original_height+Ge.cJBean_Offset));
	  else
	    g.setClip(new Rectangle2D.Float(0F,(1-fillLevel)*original_height+Ge.cJBean_Offset,original_width,original_height));
	  break;
	case Ge.DIRECTION_RIGHT:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(fillLevel*original_width+Ge.cJBean_Offset,0F,original_width,original_height));
	  else
	    g.setClip(new Rectangle2D.Float(0F,0F,fillLevel*width+Ge.cJBean_Offset,height));
	  break;
	case Ge.DIRECTION_LEFT:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,0F,(1-fillLevel)*original_width+Ge.cJBean_Offset,original_height));
	  else
	    g.setClip(new Rectangle2D.Float((1-fillLevel)*original_width+Ge.cJBean_Offset,0F,original_width,original_height));
	  break;
	}
      }
    {
    int fcolor = GeColor.getDrawtype(34, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    g.setColor(GeColor.getColor( fcolor));
    g.fill( shapes[0]);
    g.setColor(GeColor.shiftColor( fcolor, -2, colorInverse));
    g.fill( shapes[1]);
    g.setColor(GeColor.shiftColor( fcolor, 2, colorInverse));
    g.fill( shapes[2]);
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[0]);
    }
    g.setStroke( new BasicStroke(1F, BasicStroke.CAP_BUTT, BasicStroke.JOIN_ROUND));
    g.setColor(GeColor.getColor(39, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[3]);
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(39, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[4]);
    g.setStroke( new BasicStroke(1F, BasicStroke.CAP_BUTT, BasicStroke.JOIN_ROUND));
    g.setColor(GeColor.getColor(39, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[5]);
    }
    if ( rounds == 2)
      g.setClip(null);
    g.setTransform(save);
  }
  public Dimension getPreferredSize() { return size;}
  public Dimension getMinimumSize() { return size;}
}
protected class pwr_mbnote extends GeComponent {
  // Dimension size;
  public pwr_mbnote( JopSession session)
  {
    super( session);
    size = new Dimension( 20, 20);
  }
  public int original_width = 20;
  public int original_height = 20;
  Shape[] shapes = new Shape[] { 
    new Rectangle2D.Float(2F, 2F, 16F, 16F),
    new Polygon( new int[] { 2, 18, 16, 4, 4, 2}, new int[] { 2, 2, 4, 4, 16, 18}, 6), 
    new Polygon( new int[] { 18, 18, 16, 16, 4, 2}, new int[] { 18, 2, 4, 16, 16, 18}, 6), 
    new Line2D.Float( 10F, 13.772F, 10F, 13F),
    new Line2D.Float( 10F, 11F, 10F, 6F),
  };
  public void paintComponent(Graphics g1) {
    animationCount = 1;
    if ( !visible)
      return;
    Graphics2D g = (Graphics2D) g1;
    float width = getWidth();
    float height = getHeight();
    g.setRenderingHint(RenderingHints.KEY_ANTIALIASING,RenderingHints.VALUE_ANTIALIAS_ON);
    if ( 45.0 <= rotate && rotate < 135.0) {
      g.translate( width, 0.0); 
      g.rotate( Math.PI * rotate/180, 0.0, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else if ( 135.0 <= rotate && rotate < 225.0)
    {
      g.rotate( Math.PI * rotate/180, width/2, height/2);
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    }
    else if ( 225.0 <= rotate && rotate < 315.0)
    {
      g.translate( -height, 0.0);
      g.rotate( Math.PI * rotate/180, height, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    if ( (dd.dynType & GeDyn.mDynType_Rotate) != 0 && dd.rotate != 0) {
      g.rotate( Math.PI * dd.rotate/180, 

           (dd.x0 - getX())*original_width/width,
           (dd.y0 - getY()) * original_height / height);
    }
    AffineTransform save = g.getTransform();
    AffineTransform save_tmp;
    int rounds = 1;
    if ( fillLevel != 1F)
      rounds = 2;
    int oldColor = 0;
    for ( int i = 0; i < rounds; i++) {
      if ( rounds == 2) {
        switch ( i) {
        case 0:
	  if ( levelColorTone != GeColor.COLOR_TONE_NO) {
	    oldColor = colorTone;
	    colorTone = levelColorTone;
	  }
	  else if ( levelFillColor != GeColor.COLOR_NO) {
	    oldColor = fillColor;
	    fillColor = levelFillColor;
	  }
	  break;
        case 1:
	  if ( levelColorTone != GeColor.COLOR_TONE_NO)
	    colorTone = oldColor;
	  else if ( levelFillColor != GeColor.COLOR_NO)
	    fillColor = oldColor;
	  break;
        }
        switch ( levelDirection) {
        case Ge.DIRECTION_UP:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,fillLevel*original_height+Ge.cJBean_Offset,original_width,original_height));
	  else
	    g.setClip(new Rectangle2D.Float(0F,0F,original_width,fillLevel * original_height+Ge.cJBean_Offset));
	  break;
	case Ge.DIRECTION_DOWN:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,0F,original_width,(1-fillLevel)*original_height+Ge.cJBean_Offset));
	  else
	    g.setClip(new Rectangle2D.Float(0F,(1-fillLevel)*original_height+Ge.cJBean_Offset,original_width,original_height));
	  break;
	case Ge.DIRECTION_RIGHT:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(fillLevel*original_width+Ge.cJBean_Offset,0F,original_width,original_height));
	  else
	    g.setClip(new Rectangle2D.Float(0F,0F,fillLevel*width+Ge.cJBean_Offset,height));
	  break;
	case Ge.DIRECTION_LEFT:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,0F,(1-fillLevel)*original_width+Ge.cJBean_Offset,original_height));
	  else
	    g.setClip(new Rectangle2D.Float((1-fillLevel)*original_width+Ge.cJBean_Offset,0F,original_width,original_height));
	  break;
	}
      }
    {
    int fcolor = GeColor.getDrawtype(34, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    g.setColor(GeColor.getColor( fcolor));
    g.fill( shapes[0]);
    g.setColor(GeColor.shiftColor( fcolor, -2, colorInverse));
    g.fill( shapes[1]);
    g.setColor(GeColor.shiftColor( fcolor, 2, colorInverse));
    g.fill( shapes[2]);
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[0]);
    }
    g.setStroke( new BasicStroke(1F, BasicStroke.CAP_BUTT, BasicStroke.JOIN_ROUND));
    g.setColor(GeColor.getColor(39, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[3]);
    g.setStroke( new BasicStroke(1F, BasicStroke.CAP_BUTT, BasicStroke.JOIN_ROUND));
    g.setColor(GeColor.getColor(39, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[4]);
    }
    if ( rounds == 2)
      g.setClip(null);
    g.setTransform(save);
  }
  public Dimension getPreferredSize() { return size;}
  public Dimension getMinimumSize() { return size;}
}
protected class pwr_mbhelp extends GeComponent {
  // Dimension size;
  public pwr_mbhelp( JopSession session)
  {
    super( session);
    size = new Dimension( 20, 20);
  }
  public int original_width = 20;
  public int original_height = 20;
  Shape[] shapes = new Shape[] { 
    new Rectangle2D.Float(2F, 2F, 16F, 16F),
    new Polygon( new int[] { 2, 18, 16, 4, 4, 2}, new int[] { 2, 2, 4, 4, 16, 18}, 6), 
    new Polygon( new int[] { 18, 18, 16, 16, 4, 2}, new int[] { 18, 2, 4, 16, 16, 18}, 6), 
    new Line2D.Float( 10F, 6.772F, 10F, 6F),
    new Line2D.Float( 10.8199F, 13F, 8.81989F, 13F),
    new Polygon( new int[] { 10, 10, 9, 10}, new int[] {13, 9, 9, 9}, 4),
  };
  public void paintComponent(Graphics g1) {
    animationCount = 1;
    if ( !visible)
      return;
    Graphics2D g = (Graphics2D) g1;
    float width = getWidth();
    float height = getHeight();
    g.setRenderingHint(RenderingHints.KEY_ANTIALIASING,RenderingHints.VALUE_ANTIALIAS_ON);
    if ( 45.0 <= rotate && rotate < 135.0) {
      g.translate( width, 0.0); 
      g.rotate( Math.PI * rotate/180, 0.0, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else if ( 135.0 <= rotate && rotate < 225.0)
    {
      g.rotate( Math.PI * rotate/180, width/2, height/2);
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    }
    else if ( 225.0 <= rotate && rotate < 315.0)
    {
      g.translate( -height, 0.0);
      g.rotate( Math.PI * rotate/180, height, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    if ( (dd.dynType & GeDyn.mDynType_Rotate) != 0 && dd.rotate != 0) {
      g.rotate( Math.PI * dd.rotate/180, 

           (dd.x0 - getX())*original_width/width,
           (dd.y0 - getY()) * original_height / height);
    }
    AffineTransform save = g.getTransform();
    AffineTransform save_tmp;
    int rounds = 1;
    if ( fillLevel != 1F)
      rounds = 2;
    int oldColor = 0;
    for ( int i = 0; i < rounds; i++) {
      if ( rounds == 2) {
        switch ( i) {
        case 0:
	  if ( levelColorTone != GeColor.COLOR_TONE_NO) {
	    oldColor = colorTone;
	    colorTone = levelColorTone;
	  }
	  else if ( levelFillColor != GeColor.COLOR_NO) {
	    oldColor = fillColor;
	    fillColor = levelFillColor;
	  }
	  break;
        case 1:
	  if ( levelColorTone != GeColor.COLOR_TONE_NO)
	    colorTone = oldColor;
	  else if ( levelFillColor != GeColor.COLOR_NO)
	    fillColor = oldColor;
	  break;
        }
        switch ( levelDirection) {
        case Ge.DIRECTION_UP:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,fillLevel*original_height+Ge.cJBean_Offset,original_width,original_height));
	  else
	    g.setClip(new Rectangle2D.Float(0F,0F,original_width,fillLevel * original_height+Ge.cJBean_Offset));
	  break;
	case Ge.DIRECTION_DOWN:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,0F,original_width,(1-fillLevel)*original_height+Ge.cJBean_Offset));
	  else
	    g.setClip(new Rectangle2D.Float(0F,(1-fillLevel)*original_height+Ge.cJBean_Offset,original_width,original_height));
	  break;
	case Ge.DIRECTION_RIGHT:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(fillLevel*original_width+Ge.cJBean_Offset,0F,original_width,original_height));
	  else
	    g.setClip(new Rectangle2D.Float(0F,0F,fillLevel*width+Ge.cJBean_Offset,height));
	  break;
	case Ge.DIRECTION_LEFT:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,0F,(1-fillLevel)*original_width+Ge.cJBean_Offset,original_height));
	  else
	    g.setClip(new Rectangle2D.Float((1-fillLevel)*original_width+Ge.cJBean_Offset,0F,original_width,original_height));
	  break;
	}
      }
    {
    int fcolor = GeColor.getDrawtype(34, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    g.setColor(GeColor.getColor( fcolor));
    g.fill( shapes[0]);
    g.setColor(GeColor.shiftColor( fcolor, -2, colorInverse));
    g.fill( shapes[1]);
    g.setColor(GeColor.shiftColor( fcolor, 2, colorInverse));
    g.fill( shapes[2]);
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[0]);
    }
    g.setStroke( new BasicStroke(1F, BasicStroke.CAP_BUTT, BasicStroke.JOIN_ROUND));
    g.setColor(GeColor.getColor(39, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[3]);
    g.setStroke( new BasicStroke(1F, BasicStroke.CAP_BUTT, BasicStroke.JOIN_ROUND));
    g.setColor(GeColor.getColor(39, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[4]);
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(39, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[5]);
    }
    if ( rounds == 2)
      g.setClip(null);
    g.setTransform(save);
  }
  public Dimension getPreferredSize() { return size;}
  public Dimension getMinimumSize() { return size;}
}
protected class pwr_motor2 extends GeComponent {
  // Dimension size;
  public pwr_motor2( JopSession session)
  {
    super( session);
    size = new Dimension( 98, 74);
  }
  public int original_width = 98;
  public int original_height = 74;
  Shape[] shapes = new Shape[] { 
    new Polygon( new int[] { 80, 80, 28, 24, 24, 28, 80}, new int[] {12, 62, 62, 58, 16, 12, 12}, 7),
    new Rectangle2D.Float(24F, 46F, 56F, 12F),
    new Rectangle2D.Float(2F, 34F, 18F, 4F),
    new Polygon( new int[] { 2, 20, 20, 2, 2, 2}, new int[] { 34, 34, 34, 34, 38, 38}, 6), 
    new Polygon( new int[] { 20, 20, 20, 20, 2, 2}, new int[] { 38, 34, 34, 38, 38, 38}, 6), 
    new Rectangle2D.Float(2F, 38F, 18F, 2F),
    new Polygon( new int[] { 2, 20, 20, 2, 2, 2}, new int[] { 38, 38, 38, 38, 40, 40}, 6), 
    new Polygon( new int[] { 20, 20, 20, 20, 2, 2}, new int[] { 40, 38, 38, 40, 40, 40}, 6), 
    new Rectangle2D.Float(2F, 34F, 18F, 6F),
    new Polygon( new int[] { 80, 92, 96, 96, 92, 80, 80}, new int[] {12, 12, 14, 60, 62, 62, 12}, 7),
    new Polygon( new int[] { 80, 81, 92, 92}, new int[] { 12, 15, 15,12}, 4),
    new Polygon( new int[] { 92, 92, 95, 96}, new int[] { 12, 15, 17,14}, 4),
    new Polygon( new int[] { 96, 95, 95, 96}, new int[] { 14, 17, 61,60}, 4),
    new Polygon( new int[] { 96, 95, 92, 92}, new int[] { 60, 61, 63,62}, 4),
    new Polygon( new int[] { 92, 92, 81, 80}, new int[] { 62, 63, 63,62}, 4),
    new Polygon( new int[] { 80, 81, 81, 80}, new int[] { 62, 63, 15,12}, 4),
    new Polygon( new int[] { 36, 36, 28, 28, 38, 40, 74, 76, 86, 86, 78, 78, 36}, new int[] {62, 68, 68, 72, 72, 70, 70, 72, 72, 68, 68, 62, 62}, 13),
    new Polygon( new int[] { 24, 22, 20, 20, 22, 24, 24}, new int[] {16, 16, 18, 56, 58, 58, 16}, 7),
    new Rectangle2D.Float(34F, 2F, 20F, 10F),
    new Polygon( new int[] { 34, 54, 52, 36, 36, 34}, new int[] { 2, 2, 4, 4, 10, 12}, 6), 
    new Polygon( new int[] { 54, 54, 52, 52, 36, 34}, new int[] { 12, 2, 4, 10, 10, 12}, 6), 
    new Rectangle2D.Float(38F, 62F, 38F, 6F),
    new Line2D.Float( 80F, 18F, 24F, 18F),
    new Line2D.Float( 80F, 22F, 24F, 22F),
    new Line2D.Float( 80F, 26F, 24F, 26F),
    new Line2D.Float( 80F, 32F, 24F, 32F),
    new Line2D.Float( 80F, 42F, 24F, 42F),
    new Line2D.Float( 80F, 48F, 24F, 48F),
    new Line2D.Float( 80F, 52F, 24F, 52F),
    new Line2D.Float( 80F, 56F, 24F, 56F),
    new Rectangle2D.Float(80F, 44F, 16F, 12F),
    new Polygon( new int[] { 80, 96, 96, 92, 80, 80}, new int[] {56, 56, 60, 62, 62, 56}, 6),
    new Arc2D.Float(36F, 4F, 6F, 6F, 0F, 360F, Arc2D.PIE),
    new Arc2D.Float(46F, 4F, 6F, 6F, 0F, 360F, Arc2D.PIE),
    new Polygon( new int[] { 20, 24, 24, 22, 20, 20}, new int[] {44, 44, 58, 58, 56, 44}, 6),
    new Polygon( new int[] { 24, 22, 20, 20, 22, 24, 24}, new int[] {16, 16, 18, 56, 58, 58, 16}, 7),
    new Rectangle2D.Float(80F, 22F, 16F, 6F),
    new Polygon( new int[] { 80, 92, 96, 96, 92, 80, 80}, new int[] {12, 12, 14, 60, 62, 62, 12}, 7),
    new Polygon( new int[] { 80, 81, 92, 92}, new int[] { 12, 15, 15,12}, 4),
    new Polygon( new int[] { 92, 92, 95, 96}, new int[] { 12, 15, 17,14}, 4),
    new Polygon( new int[] { 96, 95, 95, 96}, new int[] { 14, 17, 61,60}, 4),
    new Polygon( new int[] { 96, 95, 92, 92}, new int[] { 60, 61, 63,62}, 4),
    new Polygon( new int[] { 92, 92, 81, 80}, new int[] { 62, 63, 63,62}, 4),
    new Polygon( new int[] { 80, 81, 81, 80}, new int[] { 62, 63, 15,12}, 4),
    new Line2D.Float( 20F, 36F, 2F, 36F),
  };
  public void paintComponent(Graphics g1) {
    animationCount = 1;
    if ( !visible)
      return;
    Graphics2D g = (Graphics2D) g1;
    float width = getWidth();
    float height = getHeight();
    g.setRenderingHint(RenderingHints.KEY_ANTIALIASING,RenderingHints.VALUE_ANTIALIAS_ON);
    if ( 45.0 <= rotate && rotate < 135.0) {
      g.translate( width, 0.0); 
      g.rotate( Math.PI * rotate/180, 0.0, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else if ( 135.0 <= rotate && rotate < 225.0)
    {
      g.rotate( Math.PI * rotate/180, width/2, height/2);
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    }
    else if ( 225.0 <= rotate && rotate < 315.0)
    {
      g.translate( -height, 0.0);
      g.rotate( Math.PI * rotate/180, height, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    if ( (dd.dynType & GeDyn.mDynType_Rotate) != 0 && dd.rotate != 0) {
      g.rotate( Math.PI * dd.rotate/180, 

           (dd.x0 - getX())*original_width/width,
           (dd.y0 - getY()) * original_height / height);
    }
    AffineTransform save = g.getTransform();
    AffineTransform save_tmp;
    int rounds = 1;
    if ( fillLevel != 1F)
      rounds = 2;
    int oldColor = 0;
    for ( int i = 0; i < rounds; i++) {
      if ( rounds == 2) {
        switch ( i) {
        case 0:
	  if ( levelColorTone != GeColor.COLOR_TONE_NO) {
	    oldColor = colorTone;
	    colorTone = levelColorTone;
	  }
	  else if ( levelFillColor != GeColor.COLOR_NO) {
	    oldColor = fillColor;
	    fillColor = levelFillColor;
	  }
	  break;
        case 1:
	  if ( levelColorTone != GeColor.COLOR_TONE_NO)
	    colorTone = oldColor;
	  else if ( levelFillColor != GeColor.COLOR_NO)
	    fillColor = oldColor;
	  break;
        }
        switch ( levelDirection) {
        case Ge.DIRECTION_UP:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,fillLevel*original_height+Ge.cJBean_Offset,original_width,original_height));
	  else
	    g.setClip(new Rectangle2D.Float(0F,0F,original_width,fillLevel * original_height+Ge.cJBean_Offset));
	  break;
	case Ge.DIRECTION_DOWN:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,0F,original_width,(1-fillLevel)*original_height+Ge.cJBean_Offset));
	  else
	    g.setClip(new Rectangle2D.Float(0F,(1-fillLevel)*original_height+Ge.cJBean_Offset,original_width,original_height));
	  break;
	case Ge.DIRECTION_RIGHT:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(fillLevel*original_width+Ge.cJBean_Offset,0F,original_width,original_height));
	  else
	    g.setClip(new Rectangle2D.Float(0F,0F,fillLevel*width+Ge.cJBean_Offset,height));
	  break;
	case Ge.DIRECTION_LEFT:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,0F,(1-fillLevel)*original_width+Ge.cJBean_Offset,original_height));
	  else
	    g.setClip(new Rectangle2D.Float((1-fillLevel)*original_width+Ge.cJBean_Offset,0F,original_width,original_height));
	  break;
	}
      }
    g.setColor(GeColor.getColor(215, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes[0]);
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[0]);
    {
    int fcolor = GeColor.getDrawtype(216, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    g.setColor(GeColor.getColor( fcolor));
    g.fill( shapes[1]);
    }
    {
    int fcolor = GeColor.getDrawtype(22, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    g.setColor(GeColor.getColor( fcolor));
    g.fill( shapes[2]);
    if ( shadow != 0) {
    g.setColor(GeColor.shiftColor( fcolor, -2, colorInverse));
    g.fill( shapes[3]);
    g.setColor(GeColor.shiftColor( fcolor, 2, colorInverse));
    g.fill( shapes[4]);
    }
    }
    {
    int fcolor = GeColor.getDrawtype(25, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    g.setColor(GeColor.getColor( fcolor));
    g.fill( shapes[5]);
    if ( shadow != 0) {
    g.setColor(GeColor.shiftColor( fcolor, -2, colorInverse));
    g.fill( shapes[6]);
    g.setColor(GeColor.shiftColor( fcolor, 2, colorInverse));
    g.fill( shapes[7]);
    }
    }
    {
    int fcolor = GeColor.getDrawtype(32, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[8]);
    }
    g.setColor(GeColor.getColor(22, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes[9]);
    {
    int fcolor = GeColor.getDrawtype(22, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    if ( shadow != 0) {
    g.setColor(GeColor.shiftColor( fcolor, -2, colorInverse));
    g.fill( shapes[10]);
    g.setColor(GeColor.shiftColor( fcolor, -2, colorInverse));
    g.fill( shapes[11]);
    g.setColor(GeColor.shiftColor( fcolor, 2, colorInverse));
    g.fill( shapes[12]);
    g.setColor(GeColor.shiftColor( fcolor, 2, colorInverse));
    g.fill( shapes[13]);
    g.setColor(GeColor.shiftColor( fcolor, 2, colorInverse));
    g.fill( shapes[14]);
    g.setColor(GeColor.shiftColor( fcolor, -2, colorInverse));
    g.fill( shapes[15]);
    }
    }
    g.setColor(GeColor.getColor(26, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes[16]);
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[16]);
    g.setColor(GeColor.getColor(22, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes[17]);
    {
    int fcolor = GeColor.getDrawtype(56, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    g.setColor(GeColor.getColor( fcolor));
    g.fill( shapes[18]);
    if ( shadow != 0) {
    g.setColor(GeColor.shiftColor( fcolor, -2, colorInverse));
    g.fill( shapes[19]);
    g.setColor(GeColor.shiftColor( fcolor, 2, colorInverse));
    g.fill( shapes[20]);
    }
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[18]);
    }
    {
    int fcolor = GeColor.getDrawtype(28, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    g.setColor(GeColor.getColor( fcolor));
    g.fill( shapes[21]);
    }
    g.setStroke( new BasicStroke(1F, BasicStroke.CAP_BUTT, BasicStroke.JOIN_ROUND));
    g.setColor(GeColor.getColor(219, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[22]);
    g.setStroke( new BasicStroke(1F, BasicStroke.CAP_BUTT, BasicStroke.JOIN_ROUND));
    g.setColor(GeColor.getColor(219, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[23]);
    g.setStroke( new BasicStroke(1F, BasicStroke.CAP_BUTT, BasicStroke.JOIN_ROUND));
    g.setColor(GeColor.getColor(219, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[24]);
    g.setStroke( new BasicStroke(1F, BasicStroke.CAP_BUTT, BasicStroke.JOIN_ROUND));
    g.setColor(GeColor.getColor(219, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[25]);
    g.setStroke( new BasicStroke(1F, BasicStroke.CAP_BUTT, BasicStroke.JOIN_ROUND));
    g.setColor(GeColor.getColor(219, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[26]);
    g.setStroke( new BasicStroke(1F, BasicStroke.CAP_BUTT, BasicStroke.JOIN_ROUND));
    g.setColor(GeColor.getColor(219, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[27]);
    g.setStroke( new BasicStroke(1F, BasicStroke.CAP_BUTT, BasicStroke.JOIN_ROUND));
    g.setColor(GeColor.getColor(219, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[28]);
    g.setStroke( new BasicStroke(1F, BasicStroke.CAP_BUTT, BasicStroke.JOIN_ROUND));
    g.setColor(GeColor.getColor(219, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[29]);
    {
    int fcolor = GeColor.getDrawtype(24, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    g.setColor(GeColor.getColor( fcolor));
    g.fill( shapes[30]);
    }
    g.setColor(GeColor.getColor(26, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes[31]);
    g.setColor(GeColor.getColor(53, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes[32]);
    g.setColor(GeColor.getColor(53, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes[33]);
    g.setColor(GeColor.getColor(24, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes[34]);
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[35]);
    {
    int fcolor = GeColor.getDrawtype(20, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    g.setColor(GeColor.getColor( fcolor));
    g.fill( shapes[36]);
    }
    {
    int fcolor = GeColor.getDrawtype(36, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    if ( shadow != 0) {
    g.setColor(GeColor.shiftColor( fcolor, -2, colorInverse));
    g.fill( shapes[38]);
    g.setColor(GeColor.shiftColor( fcolor, -2, colorInverse));
    g.fill( shapes[39]);
    g.setColor(GeColor.shiftColor( fcolor, 2, colorInverse));
    g.fill( shapes[40]);
    g.setColor(GeColor.shiftColor( fcolor, 2, colorInverse));
    g.fill( shapes[41]);
    g.setColor(GeColor.shiftColor( fcolor, 2, colorInverse));
    g.fill( shapes[42]);
    g.setColor(GeColor.shiftColor( fcolor, -2, colorInverse));
    g.fill( shapes[43]);
    }
    }
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[37]);
    g.setStroke( new BasicStroke(1F, BasicStroke.CAP_BUTT, BasicStroke.JOIN_ROUND));
    g.setColor(GeColor.getColor(30, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[44]);
    }
    if ( rounds == 2)
      g.setClip(null);
    g.setTransform(save);
  }
  public Dimension getPreferredSize() { return size;}
  public Dimension getMinimumSize() { return size;}
}
protected class pwr_rod extends GeComponent {
  // Dimension size;
  public pwr_rod( JopSession session)
  {
    super( session);
    size = new Dimension( 44, 10);
  }
  public int original_width = 44;
  public int original_height = 10;
  Shape[] shapes = new Shape[] { 
    new Rectangle2D.Float(2F, 2F, 40F, 6F),
    new Polygon( new int[] { 2, 42, 42, 2, 2}, new int[] {6, 6, 8, 8, 6}, 5),
    new Polygon( new int[] { 2, 42, 42, 2, 2}, new int[] {3, 3, 4, 4, 3}, 5),
    new Rectangle2D.Float(2F, 2F, 40F, 6F),
  };
  public void paintComponent(Graphics g1) {
    animationCount = 1;
    if ( !visible)
      return;
    Graphics2D g = (Graphics2D) g1;
    float width = getWidth();
    float height = getHeight();
    g.setRenderingHint(RenderingHints.KEY_ANTIALIASING,RenderingHints.VALUE_ANTIALIAS_ON);
    if ( 45.0 <= rotate && rotate < 135.0) {
      g.translate( width, 0.0); 
      g.rotate( Math.PI * rotate/180, 0.0, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else if ( 135.0 <= rotate && rotate < 225.0)
    {
      g.rotate( Math.PI * rotate/180, width/2, height/2);
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    }
    else if ( 225.0 <= rotate && rotate < 315.0)
    {
      g.translate( -height, 0.0);
      g.rotate( Math.PI * rotate/180, height, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    if ( (dd.dynType & GeDyn.mDynType_Rotate) != 0 && dd.rotate != 0) {
      g.rotate( Math.PI * dd.rotate/180, 

           (dd.x0 - getX())*original_width/width,
           (dd.y0 - getY()) * original_height / height);
    }
    AffineTransform save = g.getTransform();
    AffineTransform save_tmp;
    int rounds = 1;
    if ( fillLevel != 1F)
      rounds = 2;
    int oldColor = 0;
    for ( int i = 0; i < rounds; i++) {
      if ( rounds == 2) {
        switch ( i) {
        case 0:
	  if ( levelColorTone != GeColor.COLOR_TONE_NO) {
	    oldColor = colorTone;
	    colorTone = levelColorTone;
	  }
	  else if ( levelFillColor != GeColor.COLOR_NO) {
	    oldColor = fillColor;
	    fillColor = levelFillColor;
	  }
	  break;
        case 1:
	  if ( levelColorTone != GeColor.COLOR_TONE_NO)
	    colorTone = oldColor;
	  else if ( levelFillColor != GeColor.COLOR_NO)
	    fillColor = oldColor;
	  break;
        }
        switch ( levelDirection) {
        case Ge.DIRECTION_UP:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,fillLevel*original_height+Ge.cJBean_Offset,original_width,original_height));
	  else
	    g.setClip(new Rectangle2D.Float(0F,0F,original_width,fillLevel * original_height+Ge.cJBean_Offset));
	  break;
	case Ge.DIRECTION_DOWN:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,0F,original_width,(1-fillLevel)*original_height+Ge.cJBean_Offset));
	  else
	    g.setClip(new Rectangle2D.Float(0F,(1-fillLevel)*original_height+Ge.cJBean_Offset,original_width,original_height));
	  break;
	case Ge.DIRECTION_RIGHT:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(fillLevel*original_width+Ge.cJBean_Offset,0F,original_width,original_height));
	  else
	    g.setClip(new Rectangle2D.Float(0F,0F,fillLevel*width+Ge.cJBean_Offset,height));
	  break;
	case Ge.DIRECTION_LEFT:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,0F,(1-fillLevel)*original_width+Ge.cJBean_Offset,original_height));
	  else
	    g.setClip(new Rectangle2D.Float((1-fillLevel)*original_width+Ge.cJBean_Offset,0F,original_width,original_height));
	  break;
	}
      }
    {
    int fcolor = GeColor.getDrawtype(33, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    g.setColor(GeColor.getColor( fcolor));
    g.fill( shapes[0]);
    }
    if ( shadow != 0) {
      int fcolor = GeColor.getDrawtype(43, colorTone,
                   colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
      g.setColor(GeColor.shiftColor( fcolor,3, colorInverse));
    }
    else
      g.setColor(GeColor.getColor(43, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes[1]);
    if ( shadow != 0) {
      int fcolor = GeColor.getDrawtype(43, colorTone,
                   colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
      g.setColor(GeColor.shiftColor(fcolor,-3, colorInverse));
    }
    else
      g.setColor(GeColor.getColor(43, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes[2]);
    {
    int fcolor = GeColor.getDrawtype(43, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[3]);
    }
    }
    if ( rounds == 2)
      g.setClip(null);
    g.setTransform(save);
  }
  public Dimension getPreferredSize() { return size;}
  public Dimension getMinimumSize() { return size;}
}
protected class pwr_contactor2 extends GeComponent {
  // Dimension size;
  public pwr_contactor2( JopSession session)
  {
    super( session);
    size = new Dimension( 64, 64);
    pages = 2;
  }
  public int original_width = 64;
  public int original_height = 64;
  Shape[] shapes = new Shape[] { 
    new Rectangle2D.Float(2F, 2F, 60F, 60F),
    new Polygon( new int[] { 2, 62, 59, 5, 5, 2}, new int[] { 2, 2, 5, 5, 59, 62}, 6), 
    new Polygon( new int[] { 62, 62, 59, 59, 5, 2}, new int[] { 62, 2, 5, 59, 59, 62}, 6), 
    new Rectangle2D.Float(14F, 24F, 36F, 16F),
    new Polygon( new int[] { 14, 50, 47, 17, 17, 14}, new int[] { 24, 24, 27, 27, 37, 40}, 6), 
    new Polygon( new int[] { 50, 50, 47, 47, 17, 14}, new int[] { 40, 24, 27, 37, 37, 40}, 6), 
    new Arc2D.Float(8F, 6F, 12F, 12F, 0F, 360F, Arc2D.PIE),
    new Line2D.Float( 19.6221F, 12.1461F, 8.40804F, 12.1461F),
    new Line2D.Float( 19.6221F, 11.3451F, 8.40804F, 11.3451F),
    new Arc2D.Float(8F, 6F, 12F, 12F, 0F, 360F, Arc2D.OPEN),
    new Arc2D.Float(26F, 6F, 12F, 12F, 0F, 360F, Arc2D.PIE),
    new Line2D.Float( 37.6801F, 12.1461F, 26.466F, 12.1461F),
    new Line2D.Float( 37.6801F, 11.3451F, 26.466F, 11.3451F),
    new Arc2D.Float(26F, 6F, 12F, 12F, 0F, 360F, Arc2D.OPEN),
    new Arc2D.Float(44F, 6F, 12F, 12F, 0F, 360F, Arc2D.PIE),
    new Line2D.Float( 55.7381F, 12.1461F, 44.524F, 12.1461F),
    new Line2D.Float( 55.7381F, 11.3451F, 44.524F, 11.3451F),
    new Arc2D.Float(44F, 6F, 12F, 12F, 0F, 360F, Arc2D.OPEN),
    new Arc2D.Float(8F, 46F, 12F, 12F, 0F, 360F, Arc2D.PIE),
    new Line2D.Float( 19.6221F, 52.2285F, 8.40804F, 52.2285F),
    new Line2D.Float( 19.6221F, 51.4275F, 8.40804F, 51.4275F),
    new Arc2D.Float(8F, 46F, 12F, 12F, 0F, 360F, Arc2D.OPEN),
    new Arc2D.Float(26F, 46F, 12F, 12F, 0F, 360F, Arc2D.PIE),
    new Line2D.Float( 37.6801F, 52.2285F, 26.466F, 52.2285F),
    new Line2D.Float( 37.6801F, 51.4275F, 26.466F, 51.4275F),
    new Arc2D.Float(26F, 46F, 12F, 12F, 0F, 360F, Arc2D.OPEN),
    new Arc2D.Float(44F, 46F, 12F, 12F, 0F, 360F, Arc2D.PIE),
    new Line2D.Float( 55.7381F, 52.2285F, 44.524F, 52.2285F),
    new Line2D.Float( 55.7381F, 51.4275F, 44.524F, 51.4275F),
    new Arc2D.Float(44F, 46F, 12F, 12F, 0F, 360F, Arc2D.OPEN),
    new Polygon( new int[] { 14, 12, 12, 52, 50, 14, 14}, new int[] {40, 42, 22, 22, 24, 24, 40}, 7),
    new Polygon( new int[] { 12, 14, 50, 50, 52, 52, 12}, new int[] {42, 40, 40, 24, 22, 42, 42}, 7),
    new Polygon( new int[] { 14, 16, 52, 52, 50, 50, 14}, new int[] {40, 44, 44, 26, 24, 40, 40}, 7),
    new Rectangle2D.Float(14F, 24F, 36F, 16F),
  };
  int original_width_p2 = 64;
  int original_height_p2 = 64;
  Shape[] shapes_p2 = new Shape[] { 
    new Rectangle2D.Float(2F, 2F, 60F, 60F),
    new Polygon( new int[] { 2, 62, 59, 5, 5, 2}, new int[] { 2, 2, 5, 5, 59, 62}, 6), 
    new Polygon( new int[] { 62, 62, 59, 59, 5, 2}, new int[] { 62, 2, 5, 59, 59, 62}, 6), 
    new Rectangle2D.Float(14F, 24F, 36F, 16F),
    new Polygon( new int[] { 14, 50, 47, 17, 17, 14}, new int[] { 24, 24, 27, 27, 37, 40}, 6), 
    new Polygon( new int[] { 50, 50, 47, 47, 17, 14}, new int[] { 40, 24, 27, 37, 37, 40}, 6), 
    new Arc2D.Float(8F, 6F, 12F, 12F, 0F, 360F, Arc2D.PIE),
    new Line2D.Float( 19.6221F, 12.1461F, 8.40804F, 12.1461F),
    new Line2D.Float( 19.6221F, 11.3451F, 8.40804F, 11.3451F),
    new Arc2D.Float(8F, 6F, 12F, 12F, 0F, 360F, Arc2D.OPEN),
    new Arc2D.Float(26F, 6F, 12F, 12F, 0F, 360F, Arc2D.PIE),
    new Line2D.Float( 37.6801F, 12.1461F, 26.466F, 12.1461F),
    new Line2D.Float( 37.6801F, 11.3451F, 26.466F, 11.3451F),
    new Arc2D.Float(26F, 6F, 12F, 12F, 0F, 360F, Arc2D.OPEN),
    new Arc2D.Float(44F, 6F, 12F, 12F, 0F, 360F, Arc2D.PIE),
    new Line2D.Float( 55.7381F, 12.1461F, 44.524F, 12.1461F),
    new Line2D.Float( 55.7381F, 11.3451F, 44.524F, 11.3451F),
    new Arc2D.Float(44F, 6F, 12F, 12F, 0F, 360F, Arc2D.OPEN),
    new Arc2D.Float(8F, 46F, 12F, 12F, 0F, 360F, Arc2D.PIE),
    new Line2D.Float( 19.6221F, 52.2285F, 8.40804F, 52.2285F),
    new Line2D.Float( 19.6221F, 51.4275F, 8.40804F, 51.4275F),
    new Arc2D.Float(8F, 46F, 12F, 12F, 0F, 360F, Arc2D.OPEN),
    new Arc2D.Float(26F, 46F, 12F, 12F, 0F, 360F, Arc2D.PIE),
    new Line2D.Float( 37.6801F, 52.2285F, 26.466F, 52.2285F),
    new Line2D.Float( 37.6801F, 51.4275F, 26.466F, 51.4275F),
    new Arc2D.Float(26F, 46F, 12F, 12F, 0F, 360F, Arc2D.OPEN),
    new Arc2D.Float(44F, 46F, 12F, 12F, 0F, 360F, Arc2D.PIE),
    new Line2D.Float( 55.7381F, 52.2285F, 44.524F, 52.2285F),
    new Line2D.Float( 55.7381F, 51.4275F, 44.524F, 51.4275F),
    new Arc2D.Float(44F, 46F, 12F, 12F, 0F, 360F, Arc2D.OPEN),
    new Polygon( new int[] { 14, 12, 12, 52, 50, 14, 14}, new int[] {40, 42, 22, 22, 24, 24, 40}, 7),
    new Polygon( new int[] { 12, 14, 50, 50, 52, 52, 12}, new int[] {42, 40, 40, 24, 22, 42, 42}, 7),
    new Rectangle2D.Float(14F, 24F, 36F, 6F),
    new Rectangle2D.Float(14F, 30F, 4F, 10F),
    new Rectangle2D.Float(14F, 24F, 36F, 16F),
  };
  public void paintComponent(Graphics g1) {
    switch ( page) {
      case 2 :
        paintComponent_p2(g1);
        return;
      default:
        ;
    }
    animationCount = 1;
    if ( !visible)
      return;
    Graphics2D g = (Graphics2D) g1;
    float width = getWidth();
    float height = getHeight();
    g.setRenderingHint(RenderingHints.KEY_ANTIALIASING,RenderingHints.VALUE_ANTIALIAS_ON);
    if ( 45.0 <= rotate && rotate < 135.0) {
      g.translate( width, 0.0); 
      g.rotate( Math.PI * rotate/180, 0.0, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else if ( 135.0 <= rotate && rotate < 225.0)
    {
      g.rotate( Math.PI * rotate/180, width/2, height/2);
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    }
    else if ( 225.0 <= rotate && rotate < 315.0)
    {
      g.translate( -height, 0.0);
      g.rotate( Math.PI * rotate/180, height, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    if ( (dd.dynType & GeDyn.mDynType_Rotate) != 0 && dd.rotate != 0) {
      g.rotate( Math.PI * dd.rotate/180, 

           (dd.x0 - getX())*original_width/width,
           (dd.y0 - getY()) * original_height / height);
    }
    AffineTransform save = g.getTransform();
    AffineTransform save_tmp;
    int rounds = 1;
    if ( fillLevel != 1F)
      rounds = 2;
    int oldColor = 0;
    for ( int i = 0; i < rounds; i++) {
      if ( rounds == 2) {
        switch ( i) {
        case 0:
	  if ( levelColorTone != GeColor.COLOR_TONE_NO) {
	    oldColor = colorTone;
	    colorTone = levelColorTone;
	  }
	  else if ( levelFillColor != GeColor.COLOR_NO) {
	    oldColor = fillColor;
	    fillColor = levelFillColor;
	  }
	  break;
        case 1:
	  if ( levelColorTone != GeColor.COLOR_TONE_NO)
	    colorTone = oldColor;
	  else if ( levelFillColor != GeColor.COLOR_NO)
	    fillColor = oldColor;
	  break;
        }
        switch ( levelDirection) {
        case Ge.DIRECTION_UP:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,fillLevel*original_height+Ge.cJBean_Offset,original_width,original_height));
	  else
	    g.setClip(new Rectangle2D.Float(0F,0F,original_width,fillLevel * original_height+Ge.cJBean_Offset));
	  break;
	case Ge.DIRECTION_DOWN:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,0F,original_width,(1-fillLevel)*original_height+Ge.cJBean_Offset));
	  else
	    g.setClip(new Rectangle2D.Float(0F,(1-fillLevel)*original_height+Ge.cJBean_Offset,original_width,original_height));
	  break;
	case Ge.DIRECTION_RIGHT:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(fillLevel*original_width+Ge.cJBean_Offset,0F,original_width,original_height));
	  else
	    g.setClip(new Rectangle2D.Float(0F,0F,fillLevel*width+Ge.cJBean_Offset,height));
	  break;
	case Ge.DIRECTION_LEFT:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,0F,(1-fillLevel)*original_width+Ge.cJBean_Offset,original_height));
	  else
	    g.setClip(new Rectangle2D.Float((1-fillLevel)*original_width+Ge.cJBean_Offset,0F,original_width,original_height));
	  break;
	}
      }
    {
    int fcolor = GeColor.getDrawtype(33, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    g.setColor(GeColor.getColor( fcolor));
    g.fill( shapes[0]);
    if ( shadow != 0) {
    g.setColor(GeColor.shiftColor( fcolor, -2, colorInverse));
    g.fill( shapes[1]);
    g.setColor(GeColor.shiftColor( fcolor, 2, colorInverse));
    g.fill( shapes[2]);
    }
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[0]);
    }
    {
    int fcolor = GeColor.getDrawtype(37, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    g.setColor(GeColor.getColor( fcolor));
    g.fill( shapes[3]);
    if ( shadow != 0) {
    g.setColor(GeColor.shiftColor( fcolor, -2, colorInverse));
    g.fill( shapes[4]);
    g.setColor(GeColor.shiftColor( fcolor, 2, colorInverse));
    g.fill( shapes[5]);
    }
    }
    g.setColor(GeColor.getColor(34, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes[6]);
    g.setStroke( new BasicStroke(1F, BasicStroke.CAP_BUTT, BasicStroke.JOIN_ROUND));
    g.setColor(GeColor.getColor(31, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[7]);
    g.setStroke( new BasicStroke(1F, BasicStroke.CAP_BUTT, BasicStroke.JOIN_ROUND));
    g.setColor(GeColor.getColor(29, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[8]);
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[9]);
    g.setColor(GeColor.getColor(34, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes[10]);
    g.setStroke( new BasicStroke(1F, BasicStroke.CAP_BUTT, BasicStroke.JOIN_ROUND));
    g.setColor(GeColor.getColor(31, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[11]);
    g.setStroke( new BasicStroke(1F, BasicStroke.CAP_BUTT, BasicStroke.JOIN_ROUND));
    g.setColor(GeColor.getColor(29, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[12]);
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[13]);
    g.setColor(GeColor.getColor(34, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes[14]);
    g.setStroke( new BasicStroke(1F, BasicStroke.CAP_BUTT, BasicStroke.JOIN_ROUND));
    g.setColor(GeColor.getColor(31, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[15]);
    g.setStroke( new BasicStroke(1F, BasicStroke.CAP_BUTT, BasicStroke.JOIN_ROUND));
    g.setColor(GeColor.getColor(29, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[16]);
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[17]);
    g.setColor(GeColor.getColor(34, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes[18]);
    g.setStroke( new BasicStroke(1F, BasicStroke.CAP_BUTT, BasicStroke.JOIN_ROUND));
    g.setColor(GeColor.getColor(31, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[19]);
    g.setStroke( new BasicStroke(1F, BasicStroke.CAP_BUTT, BasicStroke.JOIN_ROUND));
    g.setColor(GeColor.getColor(29, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[20]);
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[21]);
    g.setColor(GeColor.getColor(34, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes[22]);
    g.setStroke( new BasicStroke(1F, BasicStroke.CAP_BUTT, BasicStroke.JOIN_ROUND));
    g.setColor(GeColor.getColor(31, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[23]);
    g.setStroke( new BasicStroke(1F, BasicStroke.CAP_BUTT, BasicStroke.JOIN_ROUND));
    g.setColor(GeColor.getColor(29, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[24]);
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[25]);
    g.setColor(GeColor.getColor(34, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes[26]);
    g.setStroke( new BasicStroke(1F, BasicStroke.CAP_BUTT, BasicStroke.JOIN_ROUND));
    g.setColor(GeColor.getColor(31, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[27]);
    g.setStroke( new BasicStroke(1F, BasicStroke.CAP_BUTT, BasicStroke.JOIN_ROUND));
    g.setColor(GeColor.getColor(29, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[28]);
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[29]);
    if ( shadow != 0) {
      int fcolor = GeColor.getDrawtype(33, colorTone,
                   colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
      g.setColor(GeColor.shiftColor( fcolor,2, colorInverse));
    }
    else
      g.setColor(GeColor.getColor(33, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes[30]);
    if ( shadow != 0) {
      int fcolor = GeColor.getDrawtype(33, colorTone,
                   colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
      g.setColor(GeColor.shiftColor(fcolor,-2, colorInverse));
    }
    else
      g.setColor(GeColor.getColor(33, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes[31]);
    g.setColor(GeColor.getColor(37, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes[32]);
    {
    int fcolor = GeColor.getDrawtype(37, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[33]);
    }
    }
    if ( rounds == 2)
      g.setClip(null);
    g.setTransform(save);
  }
  public void paintComponent_p2(Graphics g1) {
    animationCount = 1;
    if ( !visible)
      return;
    Graphics2D g = (Graphics2D) g1;
    float width = getWidth();
    float height = getHeight();
    g.setRenderingHint(RenderingHints.KEY_ANTIALIASING,RenderingHints.VALUE_ANTIALIAS_ON);
    if ( 45.0 <= rotate && rotate < 135.0) {
      g.translate( width, 0.0); 
      g.rotate( Math.PI * rotate/180, 0.0, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else if ( 135.0 <= rotate && rotate < 225.0)
    {
      g.rotate( Math.PI * rotate/180, width/2, height/2);
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    }
    else if ( 225.0 <= rotate && rotate < 315.0)
    {
      g.translate( -height, 0.0);
      g.rotate( Math.PI * rotate/180, height, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    if ( (dd.dynType & GeDyn.mDynType_Rotate) != 0 && dd.rotate != 0) {
      g.rotate( Math.PI * dd.rotate/180, 

           (dd.x0 - getX())*original_width/width,
           (dd.y0 - getY()) * original_height / height);
    }
    AffineTransform save = g.getTransform();
    AffineTransform save_tmp;
    int rounds = 1;
    if ( fillLevel != 1F)
      rounds = 2;
    int oldColor = 0;
    for ( int i = 0; i < rounds; i++) {
      if ( rounds == 2) {
        switch ( i) {
        case 0:
	  if ( levelColorTone != GeColor.COLOR_TONE_NO) {
	    oldColor = colorTone;
	    colorTone = levelColorTone;
	  }
	  else if ( levelFillColor != GeColor.COLOR_NO) {
	    oldColor = fillColor;
	    fillColor = levelFillColor;
	  }
	  break;
        case 1:
	  if ( levelColorTone != GeColor.COLOR_TONE_NO)
	    colorTone = oldColor;
	  else if ( levelFillColor != GeColor.COLOR_NO)
	    fillColor = oldColor;
	  break;
        }
        switch ( levelDirection) {
        case Ge.DIRECTION_UP:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,fillLevel*original_height+Ge.cJBean_Offset,original_width,original_height));
	  else
	    g.setClip(new Rectangle2D.Float(0F,0F,original_width,fillLevel * original_height+Ge.cJBean_Offset));
	  break;
	case Ge.DIRECTION_DOWN:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,0F,original_width,(1-fillLevel)*original_height+Ge.cJBean_Offset));
	  else
	    g.setClip(new Rectangle2D.Float(0F,(1-fillLevel)*original_height+Ge.cJBean_Offset,original_width,original_height));
	  break;
	case Ge.DIRECTION_RIGHT:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(fillLevel*original_width+Ge.cJBean_Offset,0F,original_width,original_height));
	  else
	    g.setClip(new Rectangle2D.Float(0F,0F,fillLevel*width+Ge.cJBean_Offset,height));
	  break;
	case Ge.DIRECTION_LEFT:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,0F,(1-fillLevel)*original_width+Ge.cJBean_Offset,original_height));
	  else
	    g.setClip(new Rectangle2D.Float((1-fillLevel)*original_width+Ge.cJBean_Offset,0F,original_width,original_height));
	  break;
	}
      }
    {
    int fcolor = GeColor.getDrawtype(33, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    g.setColor(GeColor.getColor( fcolor));
    g.fill( shapes_p2[0]);
    if ( shadow != 0) {
    g.setColor(GeColor.shiftColor( fcolor, -2, colorInverse));
    g.fill( shapes_p2[1]);
    g.setColor(GeColor.shiftColor( fcolor, 2, colorInverse));
    g.fill( shapes_p2[2]);
    }
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes_p2[0]);
    }
    {
    int fcolor = GeColor.getDrawtype(37, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    g.setColor(GeColor.getColor( fcolor));
    g.fill( shapes_p2[3]);
    if ( shadow != 0) {
    g.setColor(GeColor.shiftColor( fcolor, -2, colorInverse));
    g.fill( shapes_p2[4]);
    g.setColor(GeColor.shiftColor( fcolor, 2, colorInverse));
    g.fill( shapes_p2[5]);
    }
    }
    g.setColor(GeColor.getColor(34, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes_p2[6]);
    g.setStroke( new BasicStroke(1F, BasicStroke.CAP_BUTT, BasicStroke.JOIN_ROUND));
    g.setColor(GeColor.getColor(31, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes_p2[7]);
    g.setStroke( new BasicStroke(1F, BasicStroke.CAP_BUTT, BasicStroke.JOIN_ROUND));
    g.setColor(GeColor.getColor(29, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes_p2[8]);
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes_p2[9]);
    g.setColor(GeColor.getColor(34, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes_p2[10]);
    g.setStroke( new BasicStroke(1F, BasicStroke.CAP_BUTT, BasicStroke.JOIN_ROUND));
    g.setColor(GeColor.getColor(31, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes_p2[11]);
    g.setStroke( new BasicStroke(1F, BasicStroke.CAP_BUTT, BasicStroke.JOIN_ROUND));
    g.setColor(GeColor.getColor(29, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes_p2[12]);
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes_p2[13]);
    g.setColor(GeColor.getColor(34, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes_p2[14]);
    g.setStroke( new BasicStroke(1F, BasicStroke.CAP_BUTT, BasicStroke.JOIN_ROUND));
    g.setColor(GeColor.getColor(31, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes_p2[15]);
    g.setStroke( new BasicStroke(1F, BasicStroke.CAP_BUTT, BasicStroke.JOIN_ROUND));
    g.setColor(GeColor.getColor(29, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes_p2[16]);
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes_p2[17]);
    g.setColor(GeColor.getColor(34, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes_p2[18]);
    g.setStroke( new BasicStroke(1F, BasicStroke.CAP_BUTT, BasicStroke.JOIN_ROUND));
    g.setColor(GeColor.getColor(31, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes_p2[19]);
    g.setStroke( new BasicStroke(1F, BasicStroke.CAP_BUTT, BasicStroke.JOIN_ROUND));
    g.setColor(GeColor.getColor(29, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes_p2[20]);
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes_p2[21]);
    g.setColor(GeColor.getColor(34, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes_p2[22]);
    g.setStroke( new BasicStroke(1F, BasicStroke.CAP_BUTT, BasicStroke.JOIN_ROUND));
    g.setColor(GeColor.getColor(31, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes_p2[23]);
    g.setStroke( new BasicStroke(1F, BasicStroke.CAP_BUTT, BasicStroke.JOIN_ROUND));
    g.setColor(GeColor.getColor(29, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes_p2[24]);
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes_p2[25]);
    g.setColor(GeColor.getColor(34, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes_p2[26]);
    g.setStroke( new BasicStroke(1F, BasicStroke.CAP_BUTT, BasicStroke.JOIN_ROUND));
    g.setColor(GeColor.getColor(31, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes_p2[27]);
    g.setStroke( new BasicStroke(1F, BasicStroke.CAP_BUTT, BasicStroke.JOIN_ROUND));
    g.setColor(GeColor.getColor(29, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes_p2[28]);
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes_p2[29]);
    if ( shadow != 0) {
      int fcolor = GeColor.getDrawtype(33, colorTone,
                   colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
      g.setColor(GeColor.shiftColor( fcolor,2, colorInverse));
    }
    else
      g.setColor(GeColor.getColor(33, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes_p2[30]);
    if ( shadow != 0) {
      int fcolor = GeColor.getDrawtype(33, colorTone,
                   colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
      g.setColor(GeColor.shiftColor(fcolor,-2, colorInverse));
    }
    else
      g.setColor(GeColor.getColor(33, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes_p2[31]);
    {
    int fcolor = GeColor.getDrawtype(39, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    g.setColor(GeColor.getColor( fcolor));
    g.fill( shapes_p2[32]);
    }
    {
    int fcolor = GeColor.getDrawtype(39, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    g.setColor(GeColor.getColor( fcolor));
    g.fill( shapes_p2[33]);
    }
    {
    int fcolor = GeColor.getDrawtype(37, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes_p2[34]);
    }
    }
    if ( rounds == 2)
      g.setClip(null);
    g.setTransform(save);
  }
  public Dimension getPreferredSize() { return size;}
  public Dimension getMinimumSize() { return size;}
}
protected class pwr_safetyswitch extends GeComponent {
  // Dimension size;
  public pwr_safetyswitch( JopSession session)
  {
    super( session);
    size = new Dimension( 64, 64);
    pages = 2;
  }
  public int original_width = 64;
  public int original_height = 64;
  Shape[] shapes = new Shape[] { 
    new Rectangle2D.Float(2F, 2.76992F, 60F, 60F),
    new Polygon( new int[] { 2, 62, 59, 5, 5, 2}, new int[] { 3, 3, 6, 6, 60, 63}, 6), 
    new Polygon( new int[] { 62, 62, 59, 59, 5, 2}, new int[] { 63, 3, 6, 60, 60, 63}, 6), 
    new Arc2D.Float(12F, 12.7699F, 40F, 40F, 0F, 360F, Arc2D.PIE),
    new Arc2D.Float(12F, 12.7699F, 40F, 40F, 35F, 140F, Arc2D.PIE),
    new Arc2D.Float(12F, 12.7699F, 40F, 40F, 215F, 140F, Arc2D.PIE),
    new Arc2D.Float(12F, 12.7699F, 40F, 40F, -5F, 40F, Arc2D.PIE),
    new Arc2D.Float(12F, 12.7699F, 40F, 40F, 175F, 40F, Arc2D.PIE),
    new Arc2D.Float(15.2F, 15.9699F, 33.6F, 33.6F, 0F, 360F, Arc2D.PIE),
    new Arc2D.Float(12F, 12.7699F, 40F, 40F, 0F, 360F, Arc2D.PIE),
    new Polygon( new int[] { 44, 44, 44, 38, 38}, new int[] {21, 21, 47, 51, 17}, 5),
    new Rectangle2D.Float(26F, 12.7699F, 12F, 40F),
    new Polygon( new int[] { 26, 38, 35, 29, 29, 26}, new int[] { 13, 13, 16, 16, 50, 53}, 6), 
    new Polygon( new int[] { 38, 38, 35, 35, 29, 26}, new int[] { 53, 13, 16, 50, 50, 53}, 6), 
    new Line2D.Float( 32F, 26.7699F, 32F, 12.7699F),
  };
  int original_width_p2 = 64;
  int original_height_p2 = 64;
  Shape[] shapes_p2 = new Shape[] { 
    new Rectangle2D.Float(2F, 2.764F, 60F, 60F),
    new Polygon( new int[] { 2, 62, 59, 5, 5, 2}, new int[] { 3, 3, 6, 6, 60, 63}, 6), 
    new Polygon( new int[] { 62, 62, 59, 59, 5, 2}, new int[] { 63, 3, 6, 60, 60, 63}, 6), 
    new Arc2D.Float(12F, 12.764F, 40F, 40F, 0F, 360F, Arc2D.PIE),
    new Arc2D.Float(12F, 12.764F, 40F, 40F, 35F, 140F, Arc2D.PIE),
    new Arc2D.Float(12F, 12.764F, 40F, 40F, 215F, 140F, Arc2D.PIE),
    new Arc2D.Float(12F, 12.764F, 40F, 40F, -5F, 40F, Arc2D.PIE),
    new Arc2D.Float(12F, 12.764F, 40F, 40F, 175F, 40F, Arc2D.PIE),
    new Arc2D.Float(15.2F, 15.964F, 33.6F, 33.6F, 0F, 360F, Arc2D.PIE),
    new Arc2D.Float(12F, 12.764F, 40F, 40F, 0F, 360F, Arc2D.PIE),
    new Polygon( new int[] { 46, 46, 20, 16, 50}, new int[] {45, 45, 45, 39, 39}, 5),
    new Polygon( new int[] { 8, 8, 12, 18, 22, 22, 20, 20, 18, 12, 10, 10}, new int[] {43, 27, 23, 23, 27, 43, 43, 27, 25, 25, 27, 43}, 12),
    new Rectangle2D.Float(5F, 42.764F, 20F, 16F),
    new Polygon( new int[] { 5, 25, 23, 7, 7, 5}, new int[] { 43, 43, 45, 45, 57, 59}, 6), 
    new Polygon( new int[] { 25, 25, 23, 23, 7, 5}, new int[] { 59, 43, 45, 57, 57, 59}, 6), 
    new Polygon( new int[] { 14, 14, 16, 16, 17, 18, 18, 17, 13, 12, 12, 13, 14}, new int[] {50, 54, 54, 50, 50, 49, 47, 46, 46, 47, 49, 50, 50}, 13),
    new Rectangle2D.Float(12F, 26.764F, 40F, 12F),
    new Polygon( new int[] { 12, 52, 49, 15, 15, 12}, new int[] { 27, 27, 30, 30, 36, 39}, 6), 
    new Polygon( new int[] { 52, 52, 49, 49, 15, 12}, new int[] { 39, 27, 30, 36, 36, 39}, 6), 
    new Line2D.Float( 38F, 32.764F, 52F, 32.764F),
  };
  public void paintComponent(Graphics g1) {
    switch ( page) {
      case 2 :
        paintComponent_p2(g1);
        return;
      default:
        ;
    }
    animationCount = 1;
    if ( !visible)
      return;
    Graphics2D g = (Graphics2D) g1;
    float width = getWidth();
    float height = getHeight();
    g.setRenderingHint(RenderingHints.KEY_ANTIALIASING,RenderingHints.VALUE_ANTIALIAS_ON);
    if ( 45.0 <= rotate && rotate < 135.0) {
      g.translate( width, 0.0); 
      g.rotate( Math.PI * rotate/180, 0.0, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else if ( 135.0 <= rotate && rotate < 225.0)
    {
      g.rotate( Math.PI * rotate/180, width/2, height/2);
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    }
    else if ( 225.0 <= rotate && rotate < 315.0)
    {
      g.translate( -height, 0.0);
      g.rotate( Math.PI * rotate/180, height, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    if ( (dd.dynType & GeDyn.mDynType_Rotate) != 0 && dd.rotate != 0) {
      g.rotate( Math.PI * dd.rotate/180, 

           (dd.x0 - getX())*original_width/width,
           (dd.y0 - getY()) * original_height / height);
    }
    AffineTransform save = g.getTransform();
    AffineTransform save_tmp;
    int rounds = 1;
    if ( fillLevel != 1F)
      rounds = 2;
    int oldColor = 0;
    for ( int i = 0; i < rounds; i++) {
      if ( rounds == 2) {
        switch ( i) {
        case 0:
	  if ( levelColorTone != GeColor.COLOR_TONE_NO) {
	    oldColor = colorTone;
	    colorTone = levelColorTone;
	  }
	  else if ( levelFillColor != GeColor.COLOR_NO) {
	    oldColor = fillColor;
	    fillColor = levelFillColor;
	  }
	  break;
        case 1:
	  if ( levelColorTone != GeColor.COLOR_TONE_NO)
	    colorTone = oldColor;
	  else if ( levelFillColor != GeColor.COLOR_NO)
	    fillColor = oldColor;
	  break;
        }
        switch ( levelDirection) {
        case Ge.DIRECTION_UP:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,fillLevel*original_height+Ge.cJBean_Offset,original_width,original_height));
	  else
	    g.setClip(new Rectangle2D.Float(0F,0F,original_width,fillLevel * original_height+Ge.cJBean_Offset));
	  break;
	case Ge.DIRECTION_DOWN:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,0F,original_width,(1-fillLevel)*original_height+Ge.cJBean_Offset));
	  else
	    g.setClip(new Rectangle2D.Float(0F,(1-fillLevel)*original_height+Ge.cJBean_Offset,original_width,original_height));
	  break;
	case Ge.DIRECTION_RIGHT:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(fillLevel*original_width+Ge.cJBean_Offset,0F,original_width,original_height));
	  else
	    g.setClip(new Rectangle2D.Float(0F,0F,fillLevel*width+Ge.cJBean_Offset,height));
	  break;
	case Ge.DIRECTION_LEFT:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,0F,(1-fillLevel)*original_width+Ge.cJBean_Offset,original_height));
	  else
	    g.setClip(new Rectangle2D.Float((1-fillLevel)*original_width+Ge.cJBean_Offset,0F,original_width,original_height));
	  break;
	}
      }
    {
    int fcolor = GeColor.getDrawtype(33, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    g.setColor(GeColor.getColor( fcolor));
    g.fill( shapes[0]);
    if ( shadow != 0) {
    g.setColor(GeColor.shiftColor( fcolor, -2, colorInverse));
    g.fill( shapes[1]);
    g.setColor(GeColor.shiftColor( fcolor, 2, colorInverse));
    g.fill( shapes[2]);
    }
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[0]);
    }
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, textColor, dimmed));
    g.setFont(new Font("Helvetica", Font.BOLD, 10));
    g.drawString( "0",54, 35);
    if ( shadow == 0) {
    ((Arc2D)shapes[3]).setArcType(Arc2D.PIE);
    g.setColor(GeColor.getColor(36, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes[3]);
    ((Arc2D)shapes[3]).setArcType(Arc2D.OPEN);
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[3]);
    } else {
    ((Arc2D)shapes[4]).setArcType(Arc2D.PIE);
    ((Arc2D)shapes[5]).setArcType(Arc2D.PIE);
    ((Arc2D)shapes[6]).setArcType(Arc2D.PIE);
    ((Arc2D)shapes[7]).setArcType(Arc2D.PIE);
    ((Arc2D)shapes[8]).setArcType(Arc2D.PIE);
    {
    int fcolor = GeColor.getDrawtype(36, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    g.setColor(GeColor.shiftColor( fcolor, -2, colorInverse));
    g.fill( shapes[4]);
    g.setColor(GeColor.shiftColor( fcolor, 2, colorInverse));
    g.fill( shapes[5]);
    g.setColor(GeColor.getColor( fcolor));
    g.fill( shapes[6]);
    g.fill( shapes[7]);
    g.fill( shapes[8]);
    }
    ((Arc2D)shapes[9]).setArcType(Arc2D.OPEN);
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[9]);
    }
    if ( shadow != 0) {
      int fcolor = GeColor.getDrawtype(36, colorTone,
                   colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
      g.setColor(GeColor.shiftColor( fcolor,2, colorInverse));
    }
    else
      g.setColor(GeColor.getColor(36, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes[10]);
    {
    int fcolor = GeColor.getDrawtype(36, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    g.setColor(GeColor.getColor( fcolor));
    g.fill( shapes[11]);
    if ( shadow != 0) {
    g.setColor(GeColor.shiftColor( fcolor, -2, colorInverse));
    g.fill( shapes[12]);
    g.setColor(GeColor.shiftColor( fcolor, 2, colorInverse));
    g.fill( shapes[13]);
    }
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[11]);
    }
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, textColor, dimmed));
    g.setFont(new Font("Helvetica", Font.BOLD, 10));
    g.drawString( "1",29, 11);
    g.setStroke( new BasicStroke(2F, BasicStroke.CAP_BUTT, BasicStroke.JOIN_ROUND));
    g.setColor(GeColor.getColor(3, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[14]);
    }
    if ( rounds == 2)
      g.setClip(null);
    g.setTransform(save);
  }
  public void paintComponent_p2(Graphics g1) {
    animationCount = 1;
    if ( !visible)
      return;
    Graphics2D g = (Graphics2D) g1;
    float width = getWidth();
    float height = getHeight();
    g.setRenderingHint(RenderingHints.KEY_ANTIALIASING,RenderingHints.VALUE_ANTIALIAS_ON);
    if ( 45.0 <= rotate && rotate < 135.0) {
      g.translate( width, 0.0); 
      g.rotate( Math.PI * rotate/180, 0.0, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else if ( 135.0 <= rotate && rotate < 225.0)
    {
      g.rotate( Math.PI * rotate/180, width/2, height/2);
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    }
    else if ( 225.0 <= rotate && rotate < 315.0)
    {
      g.translate( -height, 0.0);
      g.rotate( Math.PI * rotate/180, height, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    if ( (dd.dynType & GeDyn.mDynType_Rotate) != 0 && dd.rotate != 0) {
      g.rotate( Math.PI * dd.rotate/180, 

           (dd.x0 - getX())*original_width/width,
           (dd.y0 - getY()) * original_height / height);
    }
    AffineTransform save = g.getTransform();
    AffineTransform save_tmp;
    int rounds = 1;
    if ( fillLevel != 1F)
      rounds = 2;
    int oldColor = 0;
    for ( int i = 0; i < rounds; i++) {
      if ( rounds == 2) {
        switch ( i) {
        case 0:
	  if ( levelColorTone != GeColor.COLOR_TONE_NO) {
	    oldColor = colorTone;
	    colorTone = levelColorTone;
	  }
	  else if ( levelFillColor != GeColor.COLOR_NO) {
	    oldColor = fillColor;
	    fillColor = levelFillColor;
	  }
	  break;
        case 1:
	  if ( levelColorTone != GeColor.COLOR_TONE_NO)
	    colorTone = oldColor;
	  else if ( levelFillColor != GeColor.COLOR_NO)
	    fillColor = oldColor;
	  break;
        }
        switch ( levelDirection) {
        case Ge.DIRECTION_UP:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,fillLevel*original_height+Ge.cJBean_Offset,original_width,original_height));
	  else
	    g.setClip(new Rectangle2D.Float(0F,0F,original_width,fillLevel * original_height+Ge.cJBean_Offset));
	  break;
	case Ge.DIRECTION_DOWN:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,0F,original_width,(1-fillLevel)*original_height+Ge.cJBean_Offset));
	  else
	    g.setClip(new Rectangle2D.Float(0F,(1-fillLevel)*original_height+Ge.cJBean_Offset,original_width,original_height));
	  break;
	case Ge.DIRECTION_RIGHT:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(fillLevel*original_width+Ge.cJBean_Offset,0F,original_width,original_height));
	  else
	    g.setClip(new Rectangle2D.Float(0F,0F,fillLevel*width+Ge.cJBean_Offset,height));
	  break;
	case Ge.DIRECTION_LEFT:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,0F,(1-fillLevel)*original_width+Ge.cJBean_Offset,original_height));
	  else
	    g.setClip(new Rectangle2D.Float((1-fillLevel)*original_width+Ge.cJBean_Offset,0F,original_width,original_height));
	  break;
	}
      }
    {
    int fcolor = GeColor.getDrawtype(33, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    g.setColor(GeColor.getColor( fcolor));
    g.fill( shapes_p2[0]);
    if ( shadow != 0) {
    g.setColor(GeColor.shiftColor( fcolor, -2, colorInverse));
    g.fill( shapes_p2[1]);
    g.setColor(GeColor.shiftColor( fcolor, 2, colorInverse));
    g.fill( shapes_p2[2]);
    }
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes_p2[0]);
    }
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, textColor, dimmed));
    g.setFont(new Font("Helvetica", Font.BOLD, 10));
    g.drawString( "0",54, 35);
    if ( shadow == 0) {
    ((Arc2D)shapes_p2[3]).setArcType(Arc2D.PIE);
    g.setColor(GeColor.getColor(36, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes_p2[3]);
    ((Arc2D)shapes_p2[3]).setArcType(Arc2D.OPEN);
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes_p2[3]);
    } else {
    ((Arc2D)shapes_p2[4]).setArcType(Arc2D.PIE);
    ((Arc2D)shapes_p2[5]).setArcType(Arc2D.PIE);
    ((Arc2D)shapes_p2[6]).setArcType(Arc2D.PIE);
    ((Arc2D)shapes_p2[7]).setArcType(Arc2D.PIE);
    ((Arc2D)shapes_p2[8]).setArcType(Arc2D.PIE);
    {
    int fcolor = GeColor.getDrawtype(36, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    g.setColor(GeColor.shiftColor( fcolor, -2, colorInverse));
    g.fill( shapes_p2[4]);
    g.setColor(GeColor.shiftColor( fcolor, 2, colorInverse));
    g.fill( shapes_p2[5]);
    g.setColor(GeColor.getColor( fcolor));
    g.fill( shapes_p2[6]);
    g.fill( shapes_p2[7]);
    g.fill( shapes_p2[8]);
    }
    ((Arc2D)shapes_p2[9]).setArcType(Arc2D.OPEN);
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes_p2[9]);
    }
    if ( shadow != 0) {
      int fcolor = GeColor.getDrawtype(36, colorTone,
                   colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
      g.setColor(GeColor.shiftColor( fcolor,2, colorInverse));
    }
    else
      g.setColor(GeColor.getColor(36, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes_p2[10]);
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, textColor, dimmed));
    g.setFont(new Font("Helvetica", Font.BOLD, 10));
    g.drawString( "1",29, 11);
    g.setColor(GeColor.getColor(176, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes_p2[11]);
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes_p2[11]);
    {
    int fcolor = GeColor.getDrawtype(176, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    g.setColor(GeColor.getColor( fcolor));
    g.fill( shapes_p2[12]);
    if ( shadow != 0) {
    g.setColor(GeColor.shiftColor( fcolor, -2, colorInverse));
    g.fill( shapes_p2[13]);
    g.setColor(GeColor.shiftColor( fcolor, 2, colorInverse));
    g.fill( shapes_p2[14]);
    }
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes_p2[12]);
    }
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.fill( shapes_p2[15]);
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes_p2[15]);
    {
    int fcolor = GeColor.getDrawtype(36, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    g.setColor(GeColor.getColor( fcolor));
    g.fill( shapes_p2[16]);
    if ( shadow != 0) {
    g.setColor(GeColor.shiftColor( fcolor, -2, colorInverse));
    g.fill( shapes_p2[17]);
    g.setColor(GeColor.shiftColor( fcolor, 2, colorInverse));
    g.fill( shapes_p2[18]);
    }
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes_p2[16]);
    }
    g.setStroke( new BasicStroke(2F, BasicStroke.CAP_BUTT, BasicStroke.JOIN_ROUND));
    g.setColor(GeColor.getColor(3, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes_p2[19]);
    }
    if ( rounds == 2)
      g.setClip(null);
    g.setTransform(save);
  }
  public Dimension getPreferredSize() { return size;}
  public Dimension getMinimumSize() { return size;}
}
protected class pwr_fananim extends GeComponent {
  // Dimension size;
  public pwr_fananim( JopSession session)
  {
    super( session);
    size = new Dimension( 16, 70);
    pages = 6;
  }
  public int original_width = 16;
  public int original_height = 70;
  Shape[] shapes = new Shape[] { 
    new Polygon( new int[] { 6, 2, 2, 5, 11, 14, 14, 10}, new int[] {30, 16, 6, 2, 2, 6, 16, 30}, 8),
    new Polygon( new int[] { 10, 14, 14, 11, 5, 2, 2, 6}, new int[] {40, 54, 64, 68, 68, 64, 54, 40}, 8),
    new Polygon( new int[] { 2, 10, 6, 2, 2}, new int[] {61, 40, 40, 54, 61}, 5),
    new Polygon( new int[] { 13, 7, 11, 14, 14, 13}, new int[] {53, 68, 68, 64, 54, 53}, 6),
    new Polygon( new int[] { 14, 6, 10, 14, 14}, new int[] {9, 30, 30, 16, 9}, 5),
    new Polygon( new int[] { 3, 9, 5, 2, 2, 3}, new int[] {17, 2, 2, 6, 16, 17}, 6),
    new Polygon( new int[] { 6, 2, 2, 5, 11, 14, 14, 10, 14, 14, 11, 5, 2, 2}, new int[] {30, 16, 6, 2, 2, 6, 16, 30, 16, 6, 2, 2, 6, 16}, 14),
    new Polygon( new int[] { 10, 14, 14, 11, 5, 2, 2, 6, 2, 2, 5, 11, 14, 14}, new int[] {40, 54, 64, 68, 68, 64, 54, 40, 54, 64, 68, 68, 64, 54}, 14),
    new Rectangle2D.Float(3F, 30F, 10F, 10F),
    new Polygon( new int[] { 3, 13, 13, 3, 3}, new int[] {32, 32, 34, 34, 32}, 5),
    new Polygon( new int[] { 3, 13, 13, 3, 3}, new int[] {37, 37, 40, 40, 37}, 5),
    new Rectangle2D.Float(3F, 30F, 10F, 10F),
  };
  int original_width_p2 = 16;
  int original_height_p2 = 70;
  Shape[] shapes_p2 = new Shape[] { 
    new Polygon( new int[] { 10, 14, 14, 11, 5, 2, 2, 6}, new int[] {38, 48, 55, 59, 61, 57, 48, 40}, 8),
    new Polygon( new int[] { 13, 7, 11, 14, 14, 13}, new int[] {46, 60, 59, 55, 48, 46}, 6),
    new Polygon( new int[] { 2, 9, 6, 2, 2}, new int[] {56, 39, 40, 48, 57}, 5),
    new Polygon( new int[] { 10, 6, 2, 2, 5, 11, 14, 14, 10}, new int[] {38, 40, 48, 57, 61, 59, 55, 48, 38}, 9),
    new Rectangle2D.Float(3F, 30F, 10F, 10F),
    new Polygon( new int[] { 3, 13, 13, 3, 3}, new int[] {32, 32, 34, 34, 32}, 5),
    new Polygon( new int[] { 3, 13, 13, 3, 3}, new int[] {37, 37, 40, 40, 37}, 5),
    new Rectangle2D.Float(3F, 30F, 10F, 10F),
    new Polygon( new int[] { 6, 2, 2, 5, 11, 14, 14, 10}, new int[] {32, 22, 15, 11, 9, 13, 22, 30}, 8),
    new Polygon( new int[] { 3, 9, 5, 2, 2, 3}, new int[] {24, 10, 11, 15, 22, 24}, 6),
    new Polygon( new int[] { 14, 7, 10, 14, 14}, new int[] {14, 31, 30, 22, 13}, 5),
    new Polygon( new int[] { 6, 10, 14, 14, 11, 5, 2, 2, 6}, new int[] {32, 30, 22, 13, 9, 11, 15, 22, 32}, 9),
  };
  int original_width_p3 = 16;
  int original_height_p3 = 70;
  Shape[] shapes_p3 = new Shape[] { 
    new Polygon( new int[] { 10, 14, 14, 12, 5, 2, 2, 6}, new int[] {35, 40, 44, 47, 50, 47, 42, 37}, 8),
    new Polygon( new int[] { 2, 10, 6, 2, 2}, new int[] {47, 35, 37, 42, 47}, 5),
    new Polygon( new int[] { 13, 7, 12, 14, 14, 13}, new int[] {39, 49, 47, 44, 40, 39}, 6),
    new Polygon( new int[] { 10, 6, 2, 2, 6, 12, 14, 14, 10}, new int[] {35, 37, 42, 47, 50, 47, 44, 40, 35}, 9),
    new Rectangle2D.Float(3F, 30F, 10F, 10F),
    new Polygon( new int[] { 3, 13, 13, 3, 3}, new int[] {32, 32, 34, 34, 32}, 5),
    new Polygon( new int[] { 3, 13, 13, 3, 3}, new int[] {37, 37, 40, 40, 37}, 5),
    new Rectangle2D.Float(3F, 30F, 10F, 10F),
    new Polygon( new int[] { 6, 2, 2, 4, 11, 14, 14, 10}, new int[] {35, 30, 26, 23, 20, 23, 28, 33}, 8),
    new Polygon( new int[] { 14, 6, 10, 14, 14}, new int[] {23, 35, 33, 28, 23}, 5),
    new Polygon( new int[] { 5, 2, 2, 4, 11, 13, 5}, new int[] {33, 30, 26, 23, 20, 22, 33}, 7),
    new Polygon( new int[] { 6, 10, 14, 14, 10, 4, 2, 2, 6}, new int[] {35, 33, 28, 23, 20, 23, 26, 30, 35}, 9),
  };
  int original_width_p4 = 16;
  int original_height_p4 = 70;
  Shape[] shapes_p4 = new Shape[] { 
    new Polygon( new int[] { 6, 2, 2, 4, 11, 14, 14, 10}, new int[] {37, 34, 30, 28, 25, 27, 32, 35}, 8),
    new Polygon( new int[] { 10, 6, 2, 2, 4, 11, 14, 14, 10}, new int[] {35, 37, 34, 30, 28, 25, 27, 32, 35}, 9),
    new Rectangle2D.Float(3F, 30F, 10F, 10F),
    new Polygon( new int[] { 3, 13, 13, 3, 3}, new int[] {32, 32, 34, 34, 32}, 5),
    new Polygon( new int[] { 3, 13, 13, 3, 3}, new int[] {37, 37, 40, 40, 37}, 5),
    new Rectangle2D.Float(3F, 30F, 10F, 10F),
    new Polygon( new int[] { 10, 14, 14, 12, 5, 2, 2, 6}, new int[] {34, 37, 41, 43, 46, 44, 39, 36}, 8),
    new Polygon( new int[] { 2, 10, 6, 2, 2}, new int[] {44, 34, 36, 39, 44}, 5),
    new Polygon( new int[] { 6, 10, 14, 14, 12, 5, 2, 2, 6}, new int[] {36, 34, 37, 41, 43, 46, 44, 39, 36}, 9),
  };
  int original_width_p5 = 16;
  int original_height_p5 = 70;
  Shape[] shapes_p5 = new Shape[] { 
    new Polygon( new int[] { 6, 2, 2, 5, 11, 14, 14, 10}, new int[] {32, 22, 15, 11, 9, 13, 22, 30}, 8),
    new Polygon( new int[] { 14, 7, 10, 14, 14}, new int[] {14, 31, 30, 22, 13}, 5),
    new Polygon( new int[] { 3, 9, 5, 2, 2, 3}, new int[] {24, 10, 11, 15, 22, 24}, 6),
    new Polygon( new int[] { 6, 10, 14, 14, 11, 5, 2, 2, 6}, new int[] {32, 30, 22, 13, 9, 11, 15, 22, 32}, 9),
    new Rectangle2D.Float(3F, 30F, 10F, 10F),
    new Polygon( new int[] { 3, 13, 13, 3, 3}, new int[] {32, 32, 34, 34, 32}, 5),
    new Polygon( new int[] { 3, 13, 13, 3, 3}, new int[] {37, 37, 40, 40, 37}, 5),
    new Rectangle2D.Float(3F, 30F, 10F, 10F),
    new Polygon( new int[] { 10, 14, 14, 11, 5, 2, 2, 6}, new int[] {38, 48, 55, 59, 61, 57, 48, 40}, 8),
    new Polygon( new int[] { 13, 7, 11, 14, 14, 13}, new int[] {46, 60, 59, 55, 48, 46}, 6),
    new Polygon( new int[] { 2, 9, 6, 2, 2}, new int[] {56, 39, 40, 48, 57}, 5),
    new Polygon( new int[] { 10, 6, 2, 2, 5, 11, 14, 14, 10}, new int[] {38, 40, 48, 57, 61, 59, 55, 48, 38}, 9),
  };
  int original_width_p6 = 64;
  int original_height_p6 = 79;
  Shape[] shapes_p6 = new Shape[] { 
    new Polygon( new int[] { 6, 2, 2, 5, 11, 14, 14, 10}, new int[] {30, 16, 6, 2, 2, 6, 16, 30}, 8),
    new Polygon( new int[] { 14, 6, 10, 14, 14}, new int[] {9, 30, 30, 16, 9}, 5),
    new Polygon( new int[] { 3, 9, 5, 2, 2, 3}, new int[] {17, 2, 2, 6, 16, 17}, 6),
    new Polygon( new int[] { 6, 2, 2, 5, 11, 14, 14, 10, 14, 14, 11, 5, 2, 2}, new int[] {30, 16, 6, 2, 2, 6, 16, 30, 16, 6, 2, 2, 6, 16}, 14),
    new Polygon( new int[] { 13, 7, 11, 14, 14, 13}, new int[] {53, 68, 68, 64, 54, 53}, 6),
    new Polygon( new int[] { 10, 14, 14, 11, 5, 2, 2, 6}, new int[] {40, 54, 64, 68, 68, 64, 54, 40}, 8),
    new Polygon( new int[] { 2, 10, 6, 2, 2}, new int[] {61, 40, 40, 54, 61}, 5),
    new Polygon( new int[] { 10, 14, 14, 11, 5, 2, 2, 6, 2, 2, 5, 11, 14, 14}, new int[] {40, 54, 64, 68, 68, 64, 54, 40, 54, 64, 68, 68, 64, 54}, 14),
    new Rectangle2D.Float(3F, 30F, 10F, 10F),
    new Polygon( new int[] { 3, 13, 13, 3, 3}, new int[] {32, 32, 34, 34, 32}, 5),
    new Polygon( new int[] { 3, 13, 13, 3, 3}, new int[] {37, 37, 40, 40, 37}, 5),
    new Rectangle2D.Float(3F, 30F, 10F, 10F),
  };
  public void paintComponent(Graphics g1) {
    switch ( page) {
      case 2 :
        paintComponent_p2(g1);
        return;
      case 3 :
        paintComponent_p3(g1);
        return;
      case 4 :
        paintComponent_p4(g1);
        return;
      case 5 :
        paintComponent_p5(g1);
        return;
      case 6 :
        paintComponent_p6(g1);
        return;
      default:
        ;
    }
    animationCount = 1;
    if ( !visible)
      return;
    Graphics2D g = (Graphics2D) g1;
    float width = getWidth();
    float height = getHeight();
    g.setRenderingHint(RenderingHints.KEY_ANTIALIASING,RenderingHints.VALUE_ANTIALIAS_ON);
    if ( 45.0 <= rotate && rotate < 135.0) {
      g.translate( width, 0.0); 
      g.rotate( Math.PI * rotate/180, 0.0, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else if ( 135.0 <= rotate && rotate < 225.0)
    {
      g.rotate( Math.PI * rotate/180, width/2, height/2);
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    }
    else if ( 225.0 <= rotate && rotate < 315.0)
    {
      g.translate( -height, 0.0);
      g.rotate( Math.PI * rotate/180, height, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    if ( (dd.dynType & GeDyn.mDynType_Rotate) != 0 && dd.rotate != 0) {
      g.rotate( Math.PI * dd.rotate/180, 

           (dd.x0 - getX())*original_width/width,
           (dd.y0 - getY()) * original_height / height);
    }
    AffineTransform save = g.getTransform();
    AffineTransform save_tmp;
    int rounds = 1;
    if ( fillLevel != 1F)
      rounds = 2;
    int oldColor = 0;
    for ( int i = 0; i < rounds; i++) {
      if ( rounds == 2) {
        switch ( i) {
        case 0:
	  if ( levelColorTone != GeColor.COLOR_TONE_NO) {
	    oldColor = colorTone;
	    colorTone = levelColorTone;
	  }
	  else if ( levelFillColor != GeColor.COLOR_NO) {
	    oldColor = fillColor;
	    fillColor = levelFillColor;
	  }
	  break;
        case 1:
	  if ( levelColorTone != GeColor.COLOR_TONE_NO)
	    colorTone = oldColor;
	  else if ( levelFillColor != GeColor.COLOR_NO)
	    fillColor = oldColor;
	  break;
        }
        switch ( levelDirection) {
        case Ge.DIRECTION_UP:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,fillLevel*original_height+Ge.cJBean_Offset,original_width,original_height));
	  else
	    g.setClip(new Rectangle2D.Float(0F,0F,original_width,fillLevel * original_height+Ge.cJBean_Offset));
	  break;
	case Ge.DIRECTION_DOWN:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,0F,original_width,(1-fillLevel)*original_height+Ge.cJBean_Offset));
	  else
	    g.setClip(new Rectangle2D.Float(0F,(1-fillLevel)*original_height+Ge.cJBean_Offset,original_width,original_height));
	  break;
	case Ge.DIRECTION_RIGHT:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(fillLevel*original_width+Ge.cJBean_Offset,0F,original_width,original_height));
	  else
	    g.setClip(new Rectangle2D.Float(0F,0F,fillLevel*width+Ge.cJBean_Offset,height));
	  break;
	case Ge.DIRECTION_LEFT:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,0F,(1-fillLevel)*original_width+Ge.cJBean_Offset,original_height));
	  else
	    g.setClip(new Rectangle2D.Float((1-fillLevel)*original_width+Ge.cJBean_Offset,0F,original_width,original_height));
	  break;
	}
      }
    g.setColor(GeColor.getColor(53, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes[0]);
    g.setColor(GeColor.getColor(53, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes[1]);
    if ( shadow != 0) {
      int fcolor = GeColor.getDrawtype(53, colorTone,
                   colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
      g.setColor(GeColor.shiftColor(fcolor,-2, colorInverse));
    }
    else
      g.setColor(GeColor.getColor(53, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes[2]);
    if ( shadow != 0) {
      int fcolor = GeColor.getDrawtype(53, colorTone,
                   colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
      g.setColor(GeColor.shiftColor( fcolor,2, colorInverse));
    }
    else
      g.setColor(GeColor.getColor(53, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes[3]);
    if ( shadow != 0) {
      int fcolor = GeColor.getDrawtype(53, colorTone,
                   colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
      g.setColor(GeColor.shiftColor( fcolor,2, colorInverse));
    }
    else
      g.setColor(GeColor.getColor(53, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes[4]);
    if ( shadow != 0) {
      int fcolor = GeColor.getDrawtype(53, colorTone,
                   colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
      g.setColor(GeColor.shiftColor(fcolor,-2, colorInverse));
    }
    else
      g.setColor(GeColor.getColor(53, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes[5]);
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[6]);
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[7]);
    {
    int fcolor = GeColor.getDrawtype(53, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    g.setColor(GeColor.getColor( fcolor));
    g.fill( shapes[8]);
    }
    if ( shadow != 0) {
      int fcolor = GeColor.getDrawtype(53, colorTone,
                   colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
      g.setColor(GeColor.shiftColor(fcolor,-2, colorInverse));
    }
    else
      g.setColor(GeColor.getColor(53, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes[9]);
    if ( shadow != 0) {
      int fcolor = GeColor.getDrawtype(53, colorTone,
                   colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
      g.setColor(GeColor.shiftColor( fcolor,2, colorInverse));
    }
    else
      g.setColor(GeColor.getColor(53, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes[10]);
    {
    int fcolor = GeColor.getDrawtype(53, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[11]);
    }
    }
    if ( rounds == 2)
      g.setClip(null);
    g.setTransform(save);
  }
  public void paintComponent_p2(Graphics g1) {
    animationCount = 1;
    if ( !visible)
      return;
    Graphics2D g = (Graphics2D) g1;
    float width = getWidth();
    float height = getHeight();
    g.setRenderingHint(RenderingHints.KEY_ANTIALIASING,RenderingHints.VALUE_ANTIALIAS_ON);
    if ( 45.0 <= rotate && rotate < 135.0) {
      g.translate( width, 0.0); 
      g.rotate( Math.PI * rotate/180, 0.0, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else if ( 135.0 <= rotate && rotate < 225.0)
    {
      g.rotate( Math.PI * rotate/180, width/2, height/2);
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    }
    else if ( 225.0 <= rotate && rotate < 315.0)
    {
      g.translate( -height, 0.0);
      g.rotate( Math.PI * rotate/180, height, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    if ( (dd.dynType & GeDyn.mDynType_Rotate) != 0 && dd.rotate != 0) {
      g.rotate( Math.PI * dd.rotate/180, 

           (dd.x0 - getX())*original_width/width,
           (dd.y0 - getY()) * original_height / height);
    }
    AffineTransform save = g.getTransform();
    AffineTransform save_tmp;
    int rounds = 1;
    if ( fillLevel != 1F)
      rounds = 2;
    int oldColor = 0;
    for ( int i = 0; i < rounds; i++) {
      if ( rounds == 2) {
        switch ( i) {
        case 0:
	  if ( levelColorTone != GeColor.COLOR_TONE_NO) {
	    oldColor = colorTone;
	    colorTone = levelColorTone;
	  }
	  else if ( levelFillColor != GeColor.COLOR_NO) {
	    oldColor = fillColor;
	    fillColor = levelFillColor;
	  }
	  break;
        case 1:
	  if ( levelColorTone != GeColor.COLOR_TONE_NO)
	    colorTone = oldColor;
	  else if ( levelFillColor != GeColor.COLOR_NO)
	    fillColor = oldColor;
	  break;
        }
        switch ( levelDirection) {
        case Ge.DIRECTION_UP:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,fillLevel*original_height+Ge.cJBean_Offset,original_width,original_height));
	  else
	    g.setClip(new Rectangle2D.Float(0F,0F,original_width,fillLevel * original_height+Ge.cJBean_Offset));
	  break;
	case Ge.DIRECTION_DOWN:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,0F,original_width,(1-fillLevel)*original_height+Ge.cJBean_Offset));
	  else
	    g.setClip(new Rectangle2D.Float(0F,(1-fillLevel)*original_height+Ge.cJBean_Offset,original_width,original_height));
	  break;
	case Ge.DIRECTION_RIGHT:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(fillLevel*original_width+Ge.cJBean_Offset,0F,original_width,original_height));
	  else
	    g.setClip(new Rectangle2D.Float(0F,0F,fillLevel*width+Ge.cJBean_Offset,height));
	  break;
	case Ge.DIRECTION_LEFT:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,0F,(1-fillLevel)*original_width+Ge.cJBean_Offset,original_height));
	  else
	    g.setClip(new Rectangle2D.Float((1-fillLevel)*original_width+Ge.cJBean_Offset,0F,original_width,original_height));
	  break;
	}
      }
    g.setColor(GeColor.getColor(33, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes_p2[0]);
    if ( shadow != 0) {
      int fcolor = GeColor.getDrawtype(33, colorTone,
                   colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
      g.setColor(GeColor.shiftColor( fcolor,2, colorInverse));
    }
    else
      g.setColor(GeColor.getColor(33, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes_p2[1]);
    if ( shadow != 0) {
      int fcolor = GeColor.getDrawtype(33, colorTone,
                   colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
      g.setColor(GeColor.shiftColor( fcolor,2, colorInverse));
    }
    else
      g.setColor(GeColor.getColor(33, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes_p2[2]);
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes_p2[3]);
    {
    int fcolor = GeColor.getDrawtype(33, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    g.setColor(GeColor.getColor( fcolor));
    g.fill( shapes_p2[4]);
    }
    if ( shadow != 0) {
      int fcolor = GeColor.getDrawtype(33, colorTone,
                   colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
      g.setColor(GeColor.shiftColor(fcolor,-2, colorInverse));
    }
    else
      g.setColor(GeColor.getColor(33, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes_p2[5]);
    if ( shadow != 0) {
      int fcolor = GeColor.getDrawtype(33, colorTone,
                   colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
      g.setColor(GeColor.shiftColor( fcolor,2, colorInverse));
    }
    else
      g.setColor(GeColor.getColor(33, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes_p2[6]);
    {
    int fcolor = GeColor.getDrawtype(33, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes_p2[7]);
    }
    g.setColor(GeColor.getColor(33, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes_p2[8]);
    if ( shadow != 0) {
      int fcolor = GeColor.getDrawtype(33, colorTone,
                   colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
      g.setColor(GeColor.shiftColor( fcolor,2, colorInverse));
    }
    else
      g.setColor(GeColor.getColor(33, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes_p2[9]);
    if ( shadow != 0) {
      int fcolor = GeColor.getDrawtype(33, colorTone,
                   colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
      g.setColor(GeColor.shiftColor( fcolor,2, colorInverse));
    }
    else
      g.setColor(GeColor.getColor(33, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes_p2[10]);
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes_p2[11]);
    }
    if ( rounds == 2)
      g.setClip(null);
    g.setTransform(save);
  }
  public void paintComponent_p3(Graphics g1) {
    animationCount = 1;
    if ( !visible)
      return;
    Graphics2D g = (Graphics2D) g1;
    float width = getWidth();
    float height = getHeight();
    g.setRenderingHint(RenderingHints.KEY_ANTIALIASING,RenderingHints.VALUE_ANTIALIAS_ON);
    if ( 45.0 <= rotate && rotate < 135.0) {
      g.translate( width, 0.0); 
      g.rotate( Math.PI * rotate/180, 0.0, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else if ( 135.0 <= rotate && rotate < 225.0)
    {
      g.rotate( Math.PI * rotate/180, width/2, height/2);
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    }
    else if ( 225.0 <= rotate && rotate < 315.0)
    {
      g.translate( -height, 0.0);
      g.rotate( Math.PI * rotate/180, height, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    if ( (dd.dynType & GeDyn.mDynType_Rotate) != 0 && dd.rotate != 0) {
      g.rotate( Math.PI * dd.rotate/180, 

           (dd.x0 - getX())*original_width/width,
           (dd.y0 - getY()) * original_height / height);
    }
    AffineTransform save = g.getTransform();
    AffineTransform save_tmp;
    int rounds = 1;
    if ( fillLevel != 1F)
      rounds = 2;
    int oldColor = 0;
    for ( int i = 0; i < rounds; i++) {
      if ( rounds == 2) {
        switch ( i) {
        case 0:
	  if ( levelColorTone != GeColor.COLOR_TONE_NO) {
	    oldColor = colorTone;
	    colorTone = levelColorTone;
	  }
	  else if ( levelFillColor != GeColor.COLOR_NO) {
	    oldColor = fillColor;
	    fillColor = levelFillColor;
	  }
	  break;
        case 1:
	  if ( levelColorTone != GeColor.COLOR_TONE_NO)
	    colorTone = oldColor;
	  else if ( levelFillColor != GeColor.COLOR_NO)
	    fillColor = oldColor;
	  break;
        }
        switch ( levelDirection) {
        case Ge.DIRECTION_UP:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,fillLevel*original_height+Ge.cJBean_Offset,original_width,original_height));
	  else
	    g.setClip(new Rectangle2D.Float(0F,0F,original_width,fillLevel * original_height+Ge.cJBean_Offset));
	  break;
	case Ge.DIRECTION_DOWN:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,0F,original_width,(1-fillLevel)*original_height+Ge.cJBean_Offset));
	  else
	    g.setClip(new Rectangle2D.Float(0F,(1-fillLevel)*original_height+Ge.cJBean_Offset,original_width,original_height));
	  break;
	case Ge.DIRECTION_RIGHT:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(fillLevel*original_width+Ge.cJBean_Offset,0F,original_width,original_height));
	  else
	    g.setClip(new Rectangle2D.Float(0F,0F,fillLevel*width+Ge.cJBean_Offset,height));
	  break;
	case Ge.DIRECTION_LEFT:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,0F,(1-fillLevel)*original_width+Ge.cJBean_Offset,original_height));
	  else
	    g.setClip(new Rectangle2D.Float((1-fillLevel)*original_width+Ge.cJBean_Offset,0F,original_width,original_height));
	  break;
	}
      }
    g.setColor(GeColor.getColor(33, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes_p3[0]);
    if ( shadow != 0) {
      int fcolor = GeColor.getDrawtype(33, colorTone,
                   colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
      g.setColor(GeColor.shiftColor( fcolor,2, colorInverse));
    }
    else
      g.setColor(GeColor.getColor(33, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes_p3[1]);
    if ( shadow != 0) {
      int fcolor = GeColor.getDrawtype(33, colorTone,
                   colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
      g.setColor(GeColor.shiftColor( fcolor,2, colorInverse));
    }
    else
      g.setColor(GeColor.getColor(33, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes_p3[2]);
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes_p3[3]);
    {
    int fcolor = GeColor.getDrawtype(33, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    g.setColor(GeColor.getColor( fcolor));
    g.fill( shapes_p3[4]);
    }
    if ( shadow != 0) {
      int fcolor = GeColor.getDrawtype(33, colorTone,
                   colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
      g.setColor(GeColor.shiftColor(fcolor,-2, colorInverse));
    }
    else
      g.setColor(GeColor.getColor(33, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes_p3[5]);
    if ( shadow != 0) {
      int fcolor = GeColor.getDrawtype(33, colorTone,
                   colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
      g.setColor(GeColor.shiftColor( fcolor,2, colorInverse));
    }
    else
      g.setColor(GeColor.getColor(33, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes_p3[6]);
    {
    int fcolor = GeColor.getDrawtype(33, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes_p3[7]);
    }
    g.setColor(GeColor.getColor(33, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes_p3[8]);
    if ( shadow != 0) {
      int fcolor = GeColor.getDrawtype(33, colorTone,
                   colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
      g.setColor(GeColor.shiftColor( fcolor,2, colorInverse));
    }
    else
      g.setColor(GeColor.getColor(33, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes_p3[9]);
    if ( shadow != 0) {
      int fcolor = GeColor.getDrawtype(33, colorTone,
                   colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
      g.setColor(GeColor.shiftColor( fcolor,2, colorInverse));
    }
    else
      g.setColor(GeColor.getColor(33, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes_p3[10]);
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes_p3[11]);
    }
    if ( rounds == 2)
      g.setClip(null);
    g.setTransform(save);
  }
  public void paintComponent_p4(Graphics g1) {
    animationCount = 1;
    if ( !visible)
      return;
    Graphics2D g = (Graphics2D) g1;
    float width = getWidth();
    float height = getHeight();
    g.setRenderingHint(RenderingHints.KEY_ANTIALIASING,RenderingHints.VALUE_ANTIALIAS_ON);
    if ( 45.0 <= rotate && rotate < 135.0) {
      g.translate( width, 0.0); 
      g.rotate( Math.PI * rotate/180, 0.0, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else if ( 135.0 <= rotate && rotate < 225.0)
    {
      g.rotate( Math.PI * rotate/180, width/2, height/2);
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    }
    else if ( 225.0 <= rotate && rotate < 315.0)
    {
      g.translate( -height, 0.0);
      g.rotate( Math.PI * rotate/180, height, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    if ( (dd.dynType & GeDyn.mDynType_Rotate) != 0 && dd.rotate != 0) {
      g.rotate( Math.PI * dd.rotate/180, 

           (dd.x0 - getX())*original_width/width,
           (dd.y0 - getY()) * original_height / height);
    }
    AffineTransform save = g.getTransform();
    AffineTransform save_tmp;
    int rounds = 1;
    if ( fillLevel != 1F)
      rounds = 2;
    int oldColor = 0;
    for ( int i = 0; i < rounds; i++) {
      if ( rounds == 2) {
        switch ( i) {
        case 0:
	  if ( levelColorTone != GeColor.COLOR_TONE_NO) {
	    oldColor = colorTone;
	    colorTone = levelColorTone;
	  }
	  else if ( levelFillColor != GeColor.COLOR_NO) {
	    oldColor = fillColor;
	    fillColor = levelFillColor;
	  }
	  break;
        case 1:
	  if ( levelColorTone != GeColor.COLOR_TONE_NO)
	    colorTone = oldColor;
	  else if ( levelFillColor != GeColor.COLOR_NO)
	    fillColor = oldColor;
	  break;
        }
        switch ( levelDirection) {
        case Ge.DIRECTION_UP:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,fillLevel*original_height+Ge.cJBean_Offset,original_width,original_height));
	  else
	    g.setClip(new Rectangle2D.Float(0F,0F,original_width,fillLevel * original_height+Ge.cJBean_Offset));
	  break;
	case Ge.DIRECTION_DOWN:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,0F,original_width,(1-fillLevel)*original_height+Ge.cJBean_Offset));
	  else
	    g.setClip(new Rectangle2D.Float(0F,(1-fillLevel)*original_height+Ge.cJBean_Offset,original_width,original_height));
	  break;
	case Ge.DIRECTION_RIGHT:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(fillLevel*original_width+Ge.cJBean_Offset,0F,original_width,original_height));
	  else
	    g.setClip(new Rectangle2D.Float(0F,0F,fillLevel*width+Ge.cJBean_Offset,height));
	  break;
	case Ge.DIRECTION_LEFT:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,0F,(1-fillLevel)*original_width+Ge.cJBean_Offset,original_height));
	  else
	    g.setClip(new Rectangle2D.Float((1-fillLevel)*original_width+Ge.cJBean_Offset,0F,original_width,original_height));
	  break;
	}
      }
    g.setColor(GeColor.getColor(33, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes_p4[0]);
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes_p4[1]);
    {
    int fcolor = GeColor.getDrawtype(33, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    g.setColor(GeColor.getColor( fcolor));
    g.fill( shapes_p4[2]);
    }
    if ( shadow != 0) {
      int fcolor = GeColor.getDrawtype(33, colorTone,
                   colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
      g.setColor(GeColor.shiftColor(fcolor,-2, colorInverse));
    }
    else
      g.setColor(GeColor.getColor(33, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes_p4[3]);
    if ( shadow != 0) {
      int fcolor = GeColor.getDrawtype(33, colorTone,
                   colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
      g.setColor(GeColor.shiftColor( fcolor,2, colorInverse));
    }
    else
      g.setColor(GeColor.getColor(33, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes_p4[4]);
    {
    int fcolor = GeColor.getDrawtype(33, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes_p4[5]);
    }
    g.setColor(GeColor.getColor(33, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes_p4[6]);
    if ( shadow != 0) {
      int fcolor = GeColor.getDrawtype(33, colorTone,
                   colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
      g.setColor(GeColor.shiftColor(fcolor,-2, colorInverse));
    }
    else
      g.setColor(GeColor.getColor(33, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes_p4[7]);
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes_p4[8]);
    }
    if ( rounds == 2)
      g.setClip(null);
    g.setTransform(save);
  }
  public void paintComponent_p5(Graphics g1) {
    animationCount = 1;
    if ( !visible)
      return;
    Graphics2D g = (Graphics2D) g1;
    float width = getWidth();
    float height = getHeight();
    g.setRenderingHint(RenderingHints.KEY_ANTIALIASING,RenderingHints.VALUE_ANTIALIAS_ON);
    if ( 45.0 <= rotate && rotate < 135.0) {
      g.translate( width, 0.0); 
      g.rotate( Math.PI * rotate/180, 0.0, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else if ( 135.0 <= rotate && rotate < 225.0)
    {
      g.rotate( Math.PI * rotate/180, width/2, height/2);
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    }
    else if ( 225.0 <= rotate && rotate < 315.0)
    {
      g.translate( -height, 0.0);
      g.rotate( Math.PI * rotate/180, height, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    if ( (dd.dynType & GeDyn.mDynType_Rotate) != 0 && dd.rotate != 0) {
      g.rotate( Math.PI * dd.rotate/180, 

           (dd.x0 - getX())*original_width/width,
           (dd.y0 - getY()) * original_height / height);
    }
    AffineTransform save = g.getTransform();
    AffineTransform save_tmp;
    int rounds = 1;
    if ( fillLevel != 1F)
      rounds = 2;
    int oldColor = 0;
    for ( int i = 0; i < rounds; i++) {
      if ( rounds == 2) {
        switch ( i) {
        case 0:
	  if ( levelColorTone != GeColor.COLOR_TONE_NO) {
	    oldColor = colorTone;
	    colorTone = levelColorTone;
	  }
	  else if ( levelFillColor != GeColor.COLOR_NO) {
	    oldColor = fillColor;
	    fillColor = levelFillColor;
	  }
	  break;
        case 1:
	  if ( levelColorTone != GeColor.COLOR_TONE_NO)
	    colorTone = oldColor;
	  else if ( levelFillColor != GeColor.COLOR_NO)
	    fillColor = oldColor;
	  break;
        }
        switch ( levelDirection) {
        case Ge.DIRECTION_UP:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,fillLevel*original_height+Ge.cJBean_Offset,original_width,original_height));
	  else
	    g.setClip(new Rectangle2D.Float(0F,0F,original_width,fillLevel * original_height+Ge.cJBean_Offset));
	  break;
	case Ge.DIRECTION_DOWN:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,0F,original_width,(1-fillLevel)*original_height+Ge.cJBean_Offset));
	  else
	    g.setClip(new Rectangle2D.Float(0F,(1-fillLevel)*original_height+Ge.cJBean_Offset,original_width,original_height));
	  break;
	case Ge.DIRECTION_RIGHT:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(fillLevel*original_width+Ge.cJBean_Offset,0F,original_width,original_height));
	  else
	    g.setClip(new Rectangle2D.Float(0F,0F,fillLevel*width+Ge.cJBean_Offset,height));
	  break;
	case Ge.DIRECTION_LEFT:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,0F,(1-fillLevel)*original_width+Ge.cJBean_Offset,original_height));
	  else
	    g.setClip(new Rectangle2D.Float((1-fillLevel)*original_width+Ge.cJBean_Offset,0F,original_width,original_height));
	  break;
	}
      }
    g.setColor(GeColor.getColor(33, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes_p5[0]);
    if ( shadow != 0) {
      int fcolor = GeColor.getDrawtype(33, colorTone,
                   colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
      g.setColor(GeColor.shiftColor( fcolor,2, colorInverse));
    }
    else
      g.setColor(GeColor.getColor(33, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes_p5[1]);
    if ( shadow != 0) {
      int fcolor = GeColor.getDrawtype(33, colorTone,
                   colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
      g.setColor(GeColor.shiftColor(fcolor,-2, colorInverse));
    }
    else
      g.setColor(GeColor.getColor(33, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes_p5[2]);
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes_p5[3]);
    {
    int fcolor = GeColor.getDrawtype(33, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    g.setColor(GeColor.getColor( fcolor));
    g.fill( shapes_p5[4]);
    }
    if ( shadow != 0) {
      int fcolor = GeColor.getDrawtype(33, colorTone,
                   colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
      g.setColor(GeColor.shiftColor(fcolor,-2, colorInverse));
    }
    else
      g.setColor(GeColor.getColor(33, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes_p5[5]);
    if ( shadow != 0) {
      int fcolor = GeColor.getDrawtype(33, colorTone,
                   colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
      g.setColor(GeColor.shiftColor( fcolor,2, colorInverse));
    }
    else
      g.setColor(GeColor.getColor(33, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes_p5[6]);
    {
    int fcolor = GeColor.getDrawtype(33, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes_p5[7]);
    }
    g.setColor(GeColor.getColor(33, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes_p5[8]);
    if ( shadow != 0) {
      int fcolor = GeColor.getDrawtype(33, colorTone,
                   colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
      g.setColor(GeColor.shiftColor(fcolor,-2, colorInverse));
    }
    else
      g.setColor(GeColor.getColor(33, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes_p5[9]);
    if ( shadow != 0) {
      int fcolor = GeColor.getDrawtype(33, colorTone,
                   colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
      g.setColor(GeColor.shiftColor( fcolor,2, colorInverse));
    }
    else
      g.setColor(GeColor.getColor(33, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes_p5[10]);
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes_p5[11]);
    }
    if ( rounds == 2)
      g.setClip(null);
    g.setTransform(save);
  }
  public void paintComponent_p6(Graphics g1) {
    animationCount = 1;
    if ( !visible)
      return;
    Graphics2D g = (Graphics2D) g1;
    float width = getWidth();
    float height = getHeight();
    g.setRenderingHint(RenderingHints.KEY_ANTIALIASING,RenderingHints.VALUE_ANTIALIAS_ON);
    if ( 45.0 <= rotate && rotate < 135.0) {
      g.translate( width, 0.0); 
      g.rotate( Math.PI * rotate/180, 0.0, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else if ( 135.0 <= rotate && rotate < 225.0)
    {
      g.rotate( Math.PI * rotate/180, width/2, height/2);
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    }
    else if ( 225.0 <= rotate && rotate < 315.0)
    {
      g.translate( -height, 0.0);
      g.rotate( Math.PI * rotate/180, height, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    if ( (dd.dynType & GeDyn.mDynType_Rotate) != 0 && dd.rotate != 0) {
      g.rotate( Math.PI * dd.rotate/180, 

           (dd.x0 - getX())*original_width/width,
           (dd.y0 - getY()) * original_height / height);
    }
    AffineTransform save = g.getTransform();
    AffineTransform save_tmp;
    int rounds = 1;
    if ( fillLevel != 1F)
      rounds = 2;
    int oldColor = 0;
    for ( int i = 0; i < rounds; i++) {
      if ( rounds == 2) {
        switch ( i) {
        case 0:
	  if ( levelColorTone != GeColor.COLOR_TONE_NO) {
	    oldColor = colorTone;
	    colorTone = levelColorTone;
	  }
	  else if ( levelFillColor != GeColor.COLOR_NO) {
	    oldColor = fillColor;
	    fillColor = levelFillColor;
	  }
	  break;
        case 1:
	  if ( levelColorTone != GeColor.COLOR_TONE_NO)
	    colorTone = oldColor;
	  else if ( levelFillColor != GeColor.COLOR_NO)
	    fillColor = oldColor;
	  break;
        }
        switch ( levelDirection) {
        case Ge.DIRECTION_UP:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,fillLevel*original_height+Ge.cJBean_Offset,original_width,original_height));
	  else
	    g.setClip(new Rectangle2D.Float(0F,0F,original_width,fillLevel * original_height+Ge.cJBean_Offset));
	  break;
	case Ge.DIRECTION_DOWN:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,0F,original_width,(1-fillLevel)*original_height+Ge.cJBean_Offset));
	  else
	    g.setClip(new Rectangle2D.Float(0F,(1-fillLevel)*original_height+Ge.cJBean_Offset,original_width,original_height));
	  break;
	case Ge.DIRECTION_RIGHT:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(fillLevel*original_width+Ge.cJBean_Offset,0F,original_width,original_height));
	  else
	    g.setClip(new Rectangle2D.Float(0F,0F,fillLevel*width+Ge.cJBean_Offset,height));
	  break;
	case Ge.DIRECTION_LEFT:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,0F,(1-fillLevel)*original_width+Ge.cJBean_Offset,original_height));
	  else
	    g.setClip(new Rectangle2D.Float((1-fillLevel)*original_width+Ge.cJBean_Offset,0F,original_width,original_height));
	  break;
	}
      }
    g.setColor(GeColor.getColor(53, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes_p6[0]);
    if ( shadow != 0) {
      int fcolor = GeColor.getDrawtype(53, colorTone,
                   colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
      g.setColor(GeColor.shiftColor( fcolor,2, colorInverse));
    }
    else
      g.setColor(GeColor.getColor(53, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes_p6[1]);
    if ( shadow != 0) {
      int fcolor = GeColor.getDrawtype(53, colorTone,
                   colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
      g.setColor(GeColor.shiftColor(fcolor,-2, colorInverse));
    }
    else
      g.setColor(GeColor.getColor(53, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes_p6[2]);
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes_p6[3]);
    if ( shadow != 0) {
      int fcolor = GeColor.getDrawtype(53, colorTone,
                   colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
      g.setColor(GeColor.shiftColor( fcolor,2, colorInverse));
    }
    else
      g.setColor(GeColor.getColor(53, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes_p6[4]);
    g.setColor(GeColor.getColor(53, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes_p6[5]);
    if ( shadow != 0) {
      int fcolor = GeColor.getDrawtype(53, colorTone,
                   colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
      g.setColor(GeColor.shiftColor(fcolor,-2, colorInverse));
    }
    else
      g.setColor(GeColor.getColor(53, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes_p6[6]);
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes_p6[7]);
    {
    int fcolor = GeColor.getDrawtype(53, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    g.setColor(GeColor.getColor( fcolor));
    g.fill( shapes_p6[8]);
    }
    if ( shadow != 0) {
      int fcolor = GeColor.getDrawtype(53, colorTone,
                   colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
      g.setColor(GeColor.shiftColor(fcolor,-2, colorInverse));
    }
    else
      g.setColor(GeColor.getColor(53, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes_p6[9]);
    if ( shadow != 0) {
      int fcolor = GeColor.getDrawtype(53, colorTone,
                   colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
      g.setColor(GeColor.shiftColor( fcolor,2, colorInverse));
    }
    else
      g.setColor(GeColor.getColor(53, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes_p6[10]);
    {
    int fcolor = GeColor.getDrawtype(53, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes_p6[11]);
    }
    }
    if ( rounds == 2)
      g.setClip(null);
    g.setTransform(save);
  }
  public Dimension getPreferredSize() { return size;}
  public Dimension getMinimumSize() { return size;}
}
protected class pwr_fananim2 extends GeComponent {
  // Dimension size;
  public pwr_fananim2( JopSession session)
  {
    super( session);
    size = new Dimension( 64, 44);
    annot1Font = new Font("Helvetica", Font.BOLD, 14);
    pages = 7;
  }
  int annot1Color = 0;
  public String getAnnot1() { return annot1;}
  public void setAnnot1( String s) { annot1 = s;}
  public void setAnnot1Font( Font font) { annot1Font = font;}
  public Font getAnnot1Font() { return annot1Font;}
  public void setAnnot1Color( int color) { annot1Color = color;}
  public int original_width = 64;
  public int original_height = 44;
  Shape[] shapes = new Shape[] { 
    new Rectangle2D.Float(42F, 2F, 20F, 16F),
    new Arc2D.Float(2F, 2F, 40F, 40F, 0F, 360F, Arc2D.PIE),
    new Arc2D.Float(2F, 2F, 40F, 40F, 35F, 140F, Arc2D.PIE),
    new Arc2D.Float(2F, 2F, 40F, 40F, 215F, 140F, Arc2D.PIE),
    new Arc2D.Float(2F, 2F, 40F, 40F, -5F, 40F, Arc2D.PIE),
    new Arc2D.Float(2F, 2F, 40F, 40F, 175F, 40F, Arc2D.PIE),
    new Arc2D.Float(5.2F, 5.2F, 33.6F, 33.6F, 0F, 360F, Arc2D.PIE),
    new Arc2D.Float(2F, 2F, 40F, 40F, 0F, 360F, Arc2D.PIE),
    new Polygon( new int[] { 8, 36, 22, 8, 22, 36}, new int[] {14, 14, 38, 14, 38, 14}, 6),
  };
  int original_width_p2 = 64;
  int original_height_p2 = 44;
  Shape[] shapes_p2 = new Shape[] { 
    new Rectangle2D.Float(42F, 2F, 20F, 16F),
    new Arc2D.Float(2F, 2F, 40F, 40F, 0F, 360F, Arc2D.PIE),
    new Arc2D.Float(2F, 2F, 40F, 40F, 35F, 140F, Arc2D.PIE),
    new Arc2D.Float(2F, 2F, 40F, 40F, 215F, 140F, Arc2D.PIE),
    new Arc2D.Float(2F, 2F, 40F, 40F, -5F, 40F, Arc2D.PIE),
    new Arc2D.Float(2F, 2F, 40F, 40F, 175F, 40F, Arc2D.PIE),
    new Arc2D.Float(5.2F, 5.2F, 33.6F, 33.6F, 0F, 360F, Arc2D.PIE),
    new Arc2D.Float(2F, 2F, 40F, 40F, 0F, 360F, Arc2D.PIE),
    new Polygon( new int[] { 6, 33, 26, 6, 26, 33}, new int[] {18, 10, 37, 18, 37, 10}, 6),
  };
  int original_width_p3 = 64;
  int original_height_p3 = 44;
  Shape[] shapes_p3 = new Shape[] { 
    new Rectangle2D.Float(42F, 2F, 20F, 16F),
    new Arc2D.Float(2F, 2F, 40F, 40F, 0F, 360F, Arc2D.PIE),
    new Arc2D.Float(2F, 2F, 40F, 40F, 35F, 140F, Arc2D.PIE),
    new Arc2D.Float(2F, 2F, 40F, 40F, 215F, 140F, Arc2D.PIE),
    new Arc2D.Float(2F, 2F, 40F, 40F, -5F, 40F, Arc2D.PIE),
    new Arc2D.Float(2F, 2F, 40F, 40F, 175F, 40F, Arc2D.PIE),
    new Arc2D.Float(5.2F, 5.2F, 33.6F, 33.6F, 0F, 360F, Arc2D.PIE),
    new Arc2D.Float(2F, 2F, 40F, 40F, 0F, 360F, Arc2D.PIE),
    new Polygon( new int[] { 6, 30, 30, 6, 30, 30}, new int[] {22, 8, 35, 22, 35, 8}, 6),
  };
  int original_width_p4 = 64;
  int original_height_p4 = 44;
  Shape[] shapes_p4 = new Shape[] { 
    new Rectangle2D.Float(42F, 2F, 20F, 16F),
    new Arc2D.Float(2F, 2F, 40F, 40F, 0F, 360F, Arc2D.PIE),
    new Arc2D.Float(2F, 2F, 40F, 40F, 35F, 140F, Arc2D.PIE),
    new Arc2D.Float(2F, 2F, 40F, 40F, 215F, 140F, Arc2D.PIE),
    new Arc2D.Float(2F, 2F, 40F, 40F, -5F, 40F, Arc2D.PIE),
    new Arc2D.Float(2F, 2F, 40F, 40F, 175F, 40F, Arc2D.PIE),
    new Arc2D.Float(5.2F, 5.2F, 33.6F, 33.6F, 0F, 360F, Arc2D.PIE),
    new Arc2D.Float(2F, 2F, 40F, 40F, 0F, 360F, Arc2D.PIE),
    new Polygon( new int[] { 6, 26, 33, 6, 33, 26}, new int[] {26, 7, 34, 27, 34, 7}, 6),
  };
  int original_width_p5 = 64;
  int original_height_p5 = 44;
  Shape[] shapes_p5 = new Shape[] { 
    new Rectangle2D.Float(42F, 2F, 20F, 16F),
    new Arc2D.Float(2F, 2F, 40F, 40F, 0F, 360F, Arc2D.PIE),
    new Arc2D.Float(2F, 2F, 40F, 40F, 35F, 140F, Arc2D.PIE),
    new Arc2D.Float(2F, 2F, 40F, 40F, 215F, 140F, Arc2D.PIE),
    new Arc2D.Float(2F, 2F, 40F, 40F, -5F, 40F, Arc2D.PIE),
    new Arc2D.Float(2F, 2F, 40F, 40F, 175F, 40F, Arc2D.PIE),
    new Arc2D.Float(5.2F, 5.2F, 33.6F, 33.6F, 0F, 360F, Arc2D.PIE),
    new Arc2D.Float(2F, 2F, 40F, 40F, 0F, 360F, Arc2D.PIE),
    new Polygon( new int[] { 8, 22, 36, 8, 36, 22}, new int[] {30, 6, 30, 31, 30, 6}, 6),
  };
  int original_width_p6 = 64;
  int original_height_p6 = 44;
  Shape[] shapes_p6 = new Shape[] { 
    new Rectangle2D.Float(42F, 2F, 20F, 16F),
    new Arc2D.Float(2F, 2F, 40F, 40F, 0F, 360F, Arc2D.PIE),
    new Arc2D.Float(2F, 2F, 40F, 40F, 35F, 140F, Arc2D.PIE),
    new Arc2D.Float(2F, 2F, 40F, 40F, 215F, 140F, Arc2D.PIE),
    new Arc2D.Float(2F, 2F, 40F, 40F, -5F, 40F, Arc2D.PIE),
    new Arc2D.Float(2F, 2F, 40F, 40F, 175F, 40F, Arc2D.PIE),
    new Arc2D.Float(5.2F, 5.2F, 33.6F, 33.6F, 0F, 360F, Arc2D.PIE),
    new Arc2D.Float(2F, 2F, 40F, 40F, 0F, 360F, Arc2D.PIE),
    new Polygon( new int[] { 11, 18, 37, 11, 37, 18}, new int[] {33, 6, 26, 34, 26, 6}, 6),
  };
  int original_width_p7 = 64;
  int original_height_p7 = 44;
  Shape[] shapes_p7 = new Shape[] { 
    new Rectangle2D.Float(42F, 2F, 20F, 16F),
    new Arc2D.Float(2F, 2F, 40F, 40F, 0F, 360F, Arc2D.PIE),
    new Arc2D.Float(2F, 2F, 40F, 40F, 35F, 140F, Arc2D.PIE),
    new Arc2D.Float(2F, 2F, 40F, 40F, 215F, 140F, Arc2D.PIE),
    new Arc2D.Float(2F, 2F, 40F, 40F, -5F, 40F, Arc2D.PIE),
    new Arc2D.Float(2F, 2F, 40F, 40F, 175F, 40F, Arc2D.PIE),
    new Arc2D.Float(5.2F, 5.2F, 33.6F, 33.6F, 0F, 360F, Arc2D.PIE),
    new Arc2D.Float(2F, 2F, 40F, 40F, 0F, 360F, Arc2D.PIE),
    new Polygon( new int[] { 14, 14, 38, 15, 38, 14}, new int[] {36, 8, 22, 36, 22, 8}, 6),
  };
  public void paintComponent(Graphics g1) {
    switch ( page) {
      case 2 :
        paintComponent_p2(g1);
        return;
      case 3 :
        paintComponent_p3(g1);
        return;
      case 4 :
        paintComponent_p4(g1);
        return;
      case 5 :
        paintComponent_p5(g1);
        return;
      case 6 :
        paintComponent_p6(g1);
        return;
      case 7 :
        paintComponent_p7(g1);
        return;
      default:
        ;
    }
    animationCount = 1;
    if ( !visible)
      return;
    Graphics2D g = (Graphics2D) g1;
    float width = getWidth();
    float height = getHeight();
    g.setRenderingHint(RenderingHints.KEY_ANTIALIASING,RenderingHints.VALUE_ANTIALIAS_ON);
    if ( 45.0 <= rotate && rotate < 135.0) {
      g.translate( width, 0.0); 
      g.rotate( Math.PI * rotate/180, 0.0, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else if ( 135.0 <= rotate && rotate < 225.0)
    {
      g.rotate( Math.PI * rotate/180, width/2, height/2);
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    }
    else if ( 225.0 <= rotate && rotate < 315.0)
    {
      g.translate( -height, 0.0);
      g.rotate( Math.PI * rotate/180, height, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    if ( (dd.dynType & GeDyn.mDynType_Rotate) != 0 && dd.rotate != 0) {
      g.rotate( Math.PI * dd.rotate/180, 

           (dd.x0 - getX())*original_width/width,
           (dd.y0 - getY()) * original_height / height);
    }
    AffineTransform save = g.getTransform();
    AffineTransform save_tmp;
    int rounds = 1;
    if ( fillLevel != 1F)
      rounds = 2;
    int oldColor = 0;
    for ( int i = 0; i < rounds; i++) {
      if ( rounds == 2) {
        switch ( i) {
        case 0:
	  if ( levelColorTone != GeColor.COLOR_TONE_NO) {
	    oldColor = colorTone;
	    colorTone = levelColorTone;
	  }
	  else if ( levelFillColor != GeColor.COLOR_NO) {
	    oldColor = fillColor;
	    fillColor = levelFillColor;
	  }
	  break;
        case 1:
	  if ( levelColorTone != GeColor.COLOR_TONE_NO)
	    colorTone = oldColor;
	  else if ( levelFillColor != GeColor.COLOR_NO)
	    fillColor = oldColor;
	  break;
        }
        switch ( levelDirection) {
        case Ge.DIRECTION_UP:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,fillLevel*original_height+Ge.cJBean_Offset,original_width,original_height));
	  else
	    g.setClip(new Rectangle2D.Float(0F,0F,original_width,fillLevel * original_height+Ge.cJBean_Offset));
	  break;
	case Ge.DIRECTION_DOWN:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,0F,original_width,(1-fillLevel)*original_height+Ge.cJBean_Offset));
	  else
	    g.setClip(new Rectangle2D.Float(0F,(1-fillLevel)*original_height+Ge.cJBean_Offset,original_width,original_height));
	  break;
	case Ge.DIRECTION_RIGHT:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(fillLevel*original_width+Ge.cJBean_Offset,0F,original_width,original_height));
	  else
	    g.setClip(new Rectangle2D.Float(0F,0F,fillLevel*width+Ge.cJBean_Offset,height));
	  break;
	case Ge.DIRECTION_LEFT:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,0F,(1-fillLevel)*original_width+Ge.cJBean_Offset,original_height));
	  else
	    g.setClip(new Rectangle2D.Float((1-fillLevel)*original_width+Ge.cJBean_Offset,0F,original_width,original_height));
	  break;
	}
      }
    {
    int fcolor = GeColor.getDrawtype(300, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    g.setColor(GeColor.getColor( fcolor));
    g.fill( shapes[0]);
    }
    if ( shadow == 0) {
    ((Arc2D)shapes[1]).setArcType(Arc2D.PIE);
    g.setColor(GeColor.getColor(74, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes[1]);
    ((Arc2D)shapes[1]).setArcType(Arc2D.OPEN);
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[1]);
    } else {
    ((Arc2D)shapes[2]).setArcType(Arc2D.PIE);
    ((Arc2D)shapes[3]).setArcType(Arc2D.PIE);
    ((Arc2D)shapes[4]).setArcType(Arc2D.PIE);
    ((Arc2D)shapes[5]).setArcType(Arc2D.PIE);
    ((Arc2D)shapes[6]).setArcType(Arc2D.PIE);
    {
    int fcolor = GeColor.getDrawtype(74, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    g.setColor(GeColor.shiftColor( fcolor, -2, colorInverse));
    g.fill( shapes[2]);
    g.setColor(GeColor.shiftColor( fcolor, 2, colorInverse));
    g.fill( shapes[3]);
    g.setColor(GeColor.getColor( fcolor));
    g.fill( shapes[4]);
    g.fill( shapes[5]);
    g.fill( shapes[6]);
    }
    ((Arc2D)shapes[7]).setArcType(Arc2D.OPEN);
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[7]);
    }
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes[8]);
    g.setColor(GeColor.getColor( annot1Color , colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, textColor, dimmed));
    g.setFont( annot1Font);
    save_tmp = g.getTransform();
    g.setRenderingHint(RenderingHints.KEY_ANTIALIASING,RenderingHints.VALUE_ANTIALIAS_OFF);
    g.transform( AffineTransform.getScaleInstance( original_width/width *
      		height/original_height, 1));
    if ( annot1 != null)
      g.drawString( annot1, 42 * original_height / height * width / original_width, 16F);
    g.setTransform( save_tmp);
    }
    if ( rounds == 2)
      g.setClip(null);
    g.setTransform(save);
  }
  public void paintComponent_p2(Graphics g1) {
    animationCount = 1;
    if ( !visible)
      return;
    Graphics2D g = (Graphics2D) g1;
    float width = getWidth();
    float height = getHeight();
    g.setRenderingHint(RenderingHints.KEY_ANTIALIASING,RenderingHints.VALUE_ANTIALIAS_ON);
    if ( 45.0 <= rotate && rotate < 135.0) {
      g.translate( width, 0.0); 
      g.rotate( Math.PI * rotate/180, 0.0, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else if ( 135.0 <= rotate && rotate < 225.0)
    {
      g.rotate( Math.PI * rotate/180, width/2, height/2);
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    }
    else if ( 225.0 <= rotate && rotate < 315.0)
    {
      g.translate( -height, 0.0);
      g.rotate( Math.PI * rotate/180, height, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    if ( (dd.dynType & GeDyn.mDynType_Rotate) != 0 && dd.rotate != 0) {
      g.rotate( Math.PI * dd.rotate/180, 

           (dd.x0 - getX())*original_width/width,
           (dd.y0 - getY()) * original_height / height);
    }
    AffineTransform save = g.getTransform();
    AffineTransform save_tmp;
    int rounds = 1;
    if ( fillLevel != 1F)
      rounds = 2;
    int oldColor = 0;
    for ( int i = 0; i < rounds; i++) {
      if ( rounds == 2) {
        switch ( i) {
        case 0:
	  if ( levelColorTone != GeColor.COLOR_TONE_NO) {
	    oldColor = colorTone;
	    colorTone = levelColorTone;
	  }
	  else if ( levelFillColor != GeColor.COLOR_NO) {
	    oldColor = fillColor;
	    fillColor = levelFillColor;
	  }
	  break;
        case 1:
	  if ( levelColorTone != GeColor.COLOR_TONE_NO)
	    colorTone = oldColor;
	  else if ( levelFillColor != GeColor.COLOR_NO)
	    fillColor = oldColor;
	  break;
        }
        switch ( levelDirection) {
        case Ge.DIRECTION_UP:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,fillLevel*original_height+Ge.cJBean_Offset,original_width,original_height));
	  else
	    g.setClip(new Rectangle2D.Float(0F,0F,original_width,fillLevel * original_height+Ge.cJBean_Offset));
	  break;
	case Ge.DIRECTION_DOWN:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,0F,original_width,(1-fillLevel)*original_height+Ge.cJBean_Offset));
	  else
	    g.setClip(new Rectangle2D.Float(0F,(1-fillLevel)*original_height+Ge.cJBean_Offset,original_width,original_height));
	  break;
	case Ge.DIRECTION_RIGHT:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(fillLevel*original_width+Ge.cJBean_Offset,0F,original_width,original_height));
	  else
	    g.setClip(new Rectangle2D.Float(0F,0F,fillLevel*width+Ge.cJBean_Offset,height));
	  break;
	case Ge.DIRECTION_LEFT:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,0F,(1-fillLevel)*original_width+Ge.cJBean_Offset,original_height));
	  else
	    g.setClip(new Rectangle2D.Float((1-fillLevel)*original_width+Ge.cJBean_Offset,0F,original_width,original_height));
	  break;
	}
      }
    {
    int fcolor = GeColor.getDrawtype(300, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    g.setColor(GeColor.getColor( fcolor));
    g.fill( shapes_p2[0]);
    }
    if ( shadow == 0) {
    ((Arc2D)shapes_p2[1]).setArcType(Arc2D.PIE);
    g.setColor(GeColor.getColor(74, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes_p2[1]);
    ((Arc2D)shapes_p2[1]).setArcType(Arc2D.OPEN);
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes_p2[1]);
    } else {
    ((Arc2D)shapes_p2[2]).setArcType(Arc2D.PIE);
    ((Arc2D)shapes_p2[3]).setArcType(Arc2D.PIE);
    ((Arc2D)shapes_p2[4]).setArcType(Arc2D.PIE);
    ((Arc2D)shapes_p2[5]).setArcType(Arc2D.PIE);
    ((Arc2D)shapes_p2[6]).setArcType(Arc2D.PIE);
    {
    int fcolor = GeColor.getDrawtype(74, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    g.setColor(GeColor.shiftColor( fcolor, -2, colorInverse));
    g.fill( shapes_p2[2]);
    g.setColor(GeColor.shiftColor( fcolor, 2, colorInverse));
    g.fill( shapes_p2[3]);
    g.setColor(GeColor.getColor( fcolor));
    g.fill( shapes_p2[4]);
    g.fill( shapes_p2[5]);
    g.fill( shapes_p2[6]);
    }
    ((Arc2D)shapes_p2[7]).setArcType(Arc2D.OPEN);
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes_p2[7]);
    }
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes_p2[8]);
    }
    if ( rounds == 2)
      g.setClip(null);
    g.setTransform(save);
  }
  public void paintComponent_p3(Graphics g1) {
    animationCount = 1;
    if ( !visible)
      return;
    Graphics2D g = (Graphics2D) g1;
    float width = getWidth();
    float height = getHeight();
    g.setRenderingHint(RenderingHints.KEY_ANTIALIASING,RenderingHints.VALUE_ANTIALIAS_ON);
    if ( 45.0 <= rotate && rotate < 135.0) {
      g.translate( width, 0.0); 
      g.rotate( Math.PI * rotate/180, 0.0, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else if ( 135.0 <= rotate && rotate < 225.0)
    {
      g.rotate( Math.PI * rotate/180, width/2, height/2);
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    }
    else if ( 225.0 <= rotate && rotate < 315.0)
    {
      g.translate( -height, 0.0);
      g.rotate( Math.PI * rotate/180, height, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    if ( (dd.dynType & GeDyn.mDynType_Rotate) != 0 && dd.rotate != 0) {
      g.rotate( Math.PI * dd.rotate/180, 

           (dd.x0 - getX())*original_width/width,
           (dd.y0 - getY()) * original_height / height);
    }
    AffineTransform save = g.getTransform();
    AffineTransform save_tmp;
    int rounds = 1;
    if ( fillLevel != 1F)
      rounds = 2;
    int oldColor = 0;
    for ( int i = 0; i < rounds; i++) {
      if ( rounds == 2) {
        switch ( i) {
        case 0:
	  if ( levelColorTone != GeColor.COLOR_TONE_NO) {
	    oldColor = colorTone;
	    colorTone = levelColorTone;
	  }
	  else if ( levelFillColor != GeColor.COLOR_NO) {
	    oldColor = fillColor;
	    fillColor = levelFillColor;
	  }
	  break;
        case 1:
	  if ( levelColorTone != GeColor.COLOR_TONE_NO)
	    colorTone = oldColor;
	  else if ( levelFillColor != GeColor.COLOR_NO)
	    fillColor = oldColor;
	  break;
        }
        switch ( levelDirection) {
        case Ge.DIRECTION_UP:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,fillLevel*original_height+Ge.cJBean_Offset,original_width,original_height));
	  else
	    g.setClip(new Rectangle2D.Float(0F,0F,original_width,fillLevel * original_height+Ge.cJBean_Offset));
	  break;
	case Ge.DIRECTION_DOWN:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,0F,original_width,(1-fillLevel)*original_height+Ge.cJBean_Offset));
	  else
	    g.setClip(new Rectangle2D.Float(0F,(1-fillLevel)*original_height+Ge.cJBean_Offset,original_width,original_height));
	  break;
	case Ge.DIRECTION_RIGHT:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(fillLevel*original_width+Ge.cJBean_Offset,0F,original_width,original_height));
	  else
	    g.setClip(new Rectangle2D.Float(0F,0F,fillLevel*width+Ge.cJBean_Offset,height));
	  break;
	case Ge.DIRECTION_LEFT:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,0F,(1-fillLevel)*original_width+Ge.cJBean_Offset,original_height));
	  else
	    g.setClip(new Rectangle2D.Float((1-fillLevel)*original_width+Ge.cJBean_Offset,0F,original_width,original_height));
	  break;
	}
      }
    {
    int fcolor = GeColor.getDrawtype(300, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    g.setColor(GeColor.getColor( fcolor));
    g.fill( shapes_p3[0]);
    }
    if ( shadow == 0) {
    ((Arc2D)shapes_p3[1]).setArcType(Arc2D.PIE);
    g.setColor(GeColor.getColor(74, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes_p3[1]);
    ((Arc2D)shapes_p3[1]).setArcType(Arc2D.OPEN);
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes_p3[1]);
    } else {
    ((Arc2D)shapes_p3[2]).setArcType(Arc2D.PIE);
    ((Arc2D)shapes_p3[3]).setArcType(Arc2D.PIE);
    ((Arc2D)shapes_p3[4]).setArcType(Arc2D.PIE);
    ((Arc2D)shapes_p3[5]).setArcType(Arc2D.PIE);
    ((Arc2D)shapes_p3[6]).setArcType(Arc2D.PIE);
    {
    int fcolor = GeColor.getDrawtype(74, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    g.setColor(GeColor.shiftColor( fcolor, -2, colorInverse));
    g.fill( shapes_p3[2]);
    g.setColor(GeColor.shiftColor( fcolor, 2, colorInverse));
    g.fill( shapes_p3[3]);
    g.setColor(GeColor.getColor( fcolor));
    g.fill( shapes_p3[4]);
    g.fill( shapes_p3[5]);
    g.fill( shapes_p3[6]);
    }
    ((Arc2D)shapes_p3[7]).setArcType(Arc2D.OPEN);
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes_p3[7]);
    }
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes_p3[8]);
    }
    if ( rounds == 2)
      g.setClip(null);
    g.setTransform(save);
  }
  public void paintComponent_p4(Graphics g1) {
    animationCount = 1;
    if ( !visible)
      return;
    Graphics2D g = (Graphics2D) g1;
    float width = getWidth();
    float height = getHeight();
    g.setRenderingHint(RenderingHints.KEY_ANTIALIASING,RenderingHints.VALUE_ANTIALIAS_ON);
    if ( 45.0 <= rotate && rotate < 135.0) {
      g.translate( width, 0.0); 
      g.rotate( Math.PI * rotate/180, 0.0, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else if ( 135.0 <= rotate && rotate < 225.0)
    {
      g.rotate( Math.PI * rotate/180, width/2, height/2);
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    }
    else if ( 225.0 <= rotate && rotate < 315.0)
    {
      g.translate( -height, 0.0);
      g.rotate( Math.PI * rotate/180, height, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    if ( (dd.dynType & GeDyn.mDynType_Rotate) != 0 && dd.rotate != 0) {
      g.rotate( Math.PI * dd.rotate/180, 

           (dd.x0 - getX())*original_width/width,
           (dd.y0 - getY()) * original_height / height);
    }
    AffineTransform save = g.getTransform();
    AffineTransform save_tmp;
    int rounds = 1;
    if ( fillLevel != 1F)
      rounds = 2;
    int oldColor = 0;
    for ( int i = 0; i < rounds; i++) {
      if ( rounds == 2) {
        switch ( i) {
        case 0:
	  if ( levelColorTone != GeColor.COLOR_TONE_NO) {
	    oldColor = colorTone;
	    colorTone = levelColorTone;
	  }
	  else if ( levelFillColor != GeColor.COLOR_NO) {
	    oldColor = fillColor;
	    fillColor = levelFillColor;
	  }
	  break;
        case 1:
	  if ( levelColorTone != GeColor.COLOR_TONE_NO)
	    colorTone = oldColor;
	  else if ( levelFillColor != GeColor.COLOR_NO)
	    fillColor = oldColor;
	  break;
        }
        switch ( levelDirection) {
        case Ge.DIRECTION_UP:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,fillLevel*original_height+Ge.cJBean_Offset,original_width,original_height));
	  else
	    g.setClip(new Rectangle2D.Float(0F,0F,original_width,fillLevel * original_height+Ge.cJBean_Offset));
	  break;
	case Ge.DIRECTION_DOWN:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,0F,original_width,(1-fillLevel)*original_height+Ge.cJBean_Offset));
	  else
	    g.setClip(new Rectangle2D.Float(0F,(1-fillLevel)*original_height+Ge.cJBean_Offset,original_width,original_height));
	  break;
	case Ge.DIRECTION_RIGHT:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(fillLevel*original_width+Ge.cJBean_Offset,0F,original_width,original_height));
	  else
	    g.setClip(new Rectangle2D.Float(0F,0F,fillLevel*width+Ge.cJBean_Offset,height));
	  break;
	case Ge.DIRECTION_LEFT:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,0F,(1-fillLevel)*original_width+Ge.cJBean_Offset,original_height));
	  else
	    g.setClip(new Rectangle2D.Float((1-fillLevel)*original_width+Ge.cJBean_Offset,0F,original_width,original_height));
	  break;
	}
      }
    {
    int fcolor = GeColor.getDrawtype(300, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    g.setColor(GeColor.getColor( fcolor));
    g.fill( shapes_p4[0]);
    }
    if ( shadow == 0) {
    ((Arc2D)shapes_p4[1]).setArcType(Arc2D.PIE);
    g.setColor(GeColor.getColor(74, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes_p4[1]);
    ((Arc2D)shapes_p4[1]).setArcType(Arc2D.OPEN);
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes_p4[1]);
    } else {
    ((Arc2D)shapes_p4[2]).setArcType(Arc2D.PIE);
    ((Arc2D)shapes_p4[3]).setArcType(Arc2D.PIE);
    ((Arc2D)shapes_p4[4]).setArcType(Arc2D.PIE);
    ((Arc2D)shapes_p4[5]).setArcType(Arc2D.PIE);
    ((Arc2D)shapes_p4[6]).setArcType(Arc2D.PIE);
    {
    int fcolor = GeColor.getDrawtype(74, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    g.setColor(GeColor.shiftColor( fcolor, -2, colorInverse));
    g.fill( shapes_p4[2]);
    g.setColor(GeColor.shiftColor( fcolor, 2, colorInverse));
    g.fill( shapes_p4[3]);
    g.setColor(GeColor.getColor( fcolor));
    g.fill( shapes_p4[4]);
    g.fill( shapes_p4[5]);
    g.fill( shapes_p4[6]);
    }
    ((Arc2D)shapes_p4[7]).setArcType(Arc2D.OPEN);
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes_p4[7]);
    }
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes_p4[8]);
    }
    if ( rounds == 2)
      g.setClip(null);
    g.setTransform(save);
  }
  public void paintComponent_p5(Graphics g1) {
    animationCount = 1;
    if ( !visible)
      return;
    Graphics2D g = (Graphics2D) g1;
    float width = getWidth();
    float height = getHeight();
    g.setRenderingHint(RenderingHints.KEY_ANTIALIASING,RenderingHints.VALUE_ANTIALIAS_ON);
    if ( 45.0 <= rotate && rotate < 135.0) {
      g.translate( width, 0.0); 
      g.rotate( Math.PI * rotate/180, 0.0, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else if ( 135.0 <= rotate && rotate < 225.0)
    {
      g.rotate( Math.PI * rotate/180, width/2, height/2);
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    }
    else if ( 225.0 <= rotate && rotate < 315.0)
    {
      g.translate( -height, 0.0);
      g.rotate( Math.PI * rotate/180, height, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    if ( (dd.dynType & GeDyn.mDynType_Rotate) != 0 && dd.rotate != 0) {
      g.rotate( Math.PI * dd.rotate/180, 

           (dd.x0 - getX())*original_width/width,
           (dd.y0 - getY()) * original_height / height);
    }
    AffineTransform save = g.getTransform();
    AffineTransform save_tmp;
    int rounds = 1;
    if ( fillLevel != 1F)
      rounds = 2;
    int oldColor = 0;
    for ( int i = 0; i < rounds; i++) {
      if ( rounds == 2) {
        switch ( i) {
        case 0:
	  if ( levelColorTone != GeColor.COLOR_TONE_NO) {
	    oldColor = colorTone;
	    colorTone = levelColorTone;
	  }
	  else if ( levelFillColor != GeColor.COLOR_NO) {
	    oldColor = fillColor;
	    fillColor = levelFillColor;
	  }
	  break;
        case 1:
	  if ( levelColorTone != GeColor.COLOR_TONE_NO)
	    colorTone = oldColor;
	  else if ( levelFillColor != GeColor.COLOR_NO)
	    fillColor = oldColor;
	  break;
        }
        switch ( levelDirection) {
        case Ge.DIRECTION_UP:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,fillLevel*original_height+Ge.cJBean_Offset,original_width,original_height));
	  else
	    g.setClip(new Rectangle2D.Float(0F,0F,original_width,fillLevel * original_height+Ge.cJBean_Offset));
	  break;
	case Ge.DIRECTION_DOWN:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,0F,original_width,(1-fillLevel)*original_height+Ge.cJBean_Offset));
	  else
	    g.setClip(new Rectangle2D.Float(0F,(1-fillLevel)*original_height+Ge.cJBean_Offset,original_width,original_height));
	  break;
	case Ge.DIRECTION_RIGHT:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(fillLevel*original_width+Ge.cJBean_Offset,0F,original_width,original_height));
	  else
	    g.setClip(new Rectangle2D.Float(0F,0F,fillLevel*width+Ge.cJBean_Offset,height));
	  break;
	case Ge.DIRECTION_LEFT:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,0F,(1-fillLevel)*original_width+Ge.cJBean_Offset,original_height));
	  else
	    g.setClip(new Rectangle2D.Float((1-fillLevel)*original_width+Ge.cJBean_Offset,0F,original_width,original_height));
	  break;
	}
      }
    {
    int fcolor = GeColor.getDrawtype(300, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    g.setColor(GeColor.getColor( fcolor));
    g.fill( shapes_p5[0]);
    }
    if ( shadow == 0) {
    ((Arc2D)shapes_p5[1]).setArcType(Arc2D.PIE);
    g.setColor(GeColor.getColor(74, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes_p5[1]);
    ((Arc2D)shapes_p5[1]).setArcType(Arc2D.OPEN);
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes_p5[1]);
    } else {
    ((Arc2D)shapes_p5[2]).setArcType(Arc2D.PIE);
    ((Arc2D)shapes_p5[3]).setArcType(Arc2D.PIE);
    ((Arc2D)shapes_p5[4]).setArcType(Arc2D.PIE);
    ((Arc2D)shapes_p5[5]).setArcType(Arc2D.PIE);
    ((Arc2D)shapes_p5[6]).setArcType(Arc2D.PIE);
    {
    int fcolor = GeColor.getDrawtype(74, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    g.setColor(GeColor.shiftColor( fcolor, -2, colorInverse));
    g.fill( shapes_p5[2]);
    g.setColor(GeColor.shiftColor( fcolor, 2, colorInverse));
    g.fill( shapes_p5[3]);
    g.setColor(GeColor.getColor( fcolor));
    g.fill( shapes_p5[4]);
    g.fill( shapes_p5[5]);
    g.fill( shapes_p5[6]);
    }
    ((Arc2D)shapes_p5[7]).setArcType(Arc2D.OPEN);
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes_p5[7]);
    }
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes_p5[8]);
    }
    if ( rounds == 2)
      g.setClip(null);
    g.setTransform(save);
  }
  public void paintComponent_p6(Graphics g1) {
    animationCount = 1;
    if ( !visible)
      return;
    Graphics2D g = (Graphics2D) g1;
    float width = getWidth();
    float height = getHeight();
    g.setRenderingHint(RenderingHints.KEY_ANTIALIASING,RenderingHints.VALUE_ANTIALIAS_ON);
    if ( 45.0 <= rotate && rotate < 135.0) {
      g.translate( width, 0.0); 
      g.rotate( Math.PI * rotate/180, 0.0, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else if ( 135.0 <= rotate && rotate < 225.0)
    {
      g.rotate( Math.PI * rotate/180, width/2, height/2);
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    }
    else if ( 225.0 <= rotate && rotate < 315.0)
    {
      g.translate( -height, 0.0);
      g.rotate( Math.PI * rotate/180, height, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    if ( (dd.dynType & GeDyn.mDynType_Rotate) != 0 && dd.rotate != 0) {
      g.rotate( Math.PI * dd.rotate/180, 

           (dd.x0 - getX())*original_width/width,
           (dd.y0 - getY()) * original_height / height);
    }
    AffineTransform save = g.getTransform();
    AffineTransform save_tmp;
    int rounds = 1;
    if ( fillLevel != 1F)
      rounds = 2;
    int oldColor = 0;
    for ( int i = 0; i < rounds; i++) {
      if ( rounds == 2) {
        switch ( i) {
        case 0:
	  if ( levelColorTone != GeColor.COLOR_TONE_NO) {
	    oldColor = colorTone;
	    colorTone = levelColorTone;
	  }
	  else if ( levelFillColor != GeColor.COLOR_NO) {
	    oldColor = fillColor;
	    fillColor = levelFillColor;
	  }
	  break;
        case 1:
	  if ( levelColorTone != GeColor.COLOR_TONE_NO)
	    colorTone = oldColor;
	  else if ( levelFillColor != GeColor.COLOR_NO)
	    fillColor = oldColor;
	  break;
        }
        switch ( levelDirection) {
        case Ge.DIRECTION_UP:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,fillLevel*original_height+Ge.cJBean_Offset,original_width,original_height));
	  else
	    g.setClip(new Rectangle2D.Float(0F,0F,original_width,fillLevel * original_height+Ge.cJBean_Offset));
	  break;
	case Ge.DIRECTION_DOWN:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,0F,original_width,(1-fillLevel)*original_height+Ge.cJBean_Offset));
	  else
	    g.setClip(new Rectangle2D.Float(0F,(1-fillLevel)*original_height+Ge.cJBean_Offset,original_width,original_height));
	  break;
	case Ge.DIRECTION_RIGHT:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(fillLevel*original_width+Ge.cJBean_Offset,0F,original_width,original_height));
	  else
	    g.setClip(new Rectangle2D.Float(0F,0F,fillLevel*width+Ge.cJBean_Offset,height));
	  break;
	case Ge.DIRECTION_LEFT:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,0F,(1-fillLevel)*original_width+Ge.cJBean_Offset,original_height));
	  else
	    g.setClip(new Rectangle2D.Float((1-fillLevel)*original_width+Ge.cJBean_Offset,0F,original_width,original_height));
	  break;
	}
      }
    {
    int fcolor = GeColor.getDrawtype(300, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    g.setColor(GeColor.getColor( fcolor));
    g.fill( shapes_p6[0]);
    }
    if ( shadow == 0) {
    ((Arc2D)shapes_p6[1]).setArcType(Arc2D.PIE);
    g.setColor(GeColor.getColor(74, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes_p6[1]);
    ((Arc2D)shapes_p6[1]).setArcType(Arc2D.OPEN);
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes_p6[1]);
    } else {
    ((Arc2D)shapes_p6[2]).setArcType(Arc2D.PIE);
    ((Arc2D)shapes_p6[3]).setArcType(Arc2D.PIE);
    ((Arc2D)shapes_p6[4]).setArcType(Arc2D.PIE);
    ((Arc2D)shapes_p6[5]).setArcType(Arc2D.PIE);
    ((Arc2D)shapes_p6[6]).setArcType(Arc2D.PIE);
    {
    int fcolor = GeColor.getDrawtype(74, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    g.setColor(GeColor.shiftColor( fcolor, -2, colorInverse));
    g.fill( shapes_p6[2]);
    g.setColor(GeColor.shiftColor( fcolor, 2, colorInverse));
    g.fill( shapes_p6[3]);
    g.setColor(GeColor.getColor( fcolor));
    g.fill( shapes_p6[4]);
    g.fill( shapes_p6[5]);
    g.fill( shapes_p6[6]);
    }
    ((Arc2D)shapes_p6[7]).setArcType(Arc2D.OPEN);
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes_p6[7]);
    }
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes_p6[8]);
    }
    if ( rounds == 2)
      g.setClip(null);
    g.setTransform(save);
  }
  public void paintComponent_p7(Graphics g1) {
    animationCount = 1;
    if ( !visible)
      return;
    Graphics2D g = (Graphics2D) g1;
    float width = getWidth();
    float height = getHeight();
    g.setRenderingHint(RenderingHints.KEY_ANTIALIASING,RenderingHints.VALUE_ANTIALIAS_ON);
    if ( 45.0 <= rotate && rotate < 135.0) {
      g.translate( width, 0.0); 
      g.rotate( Math.PI * rotate/180, 0.0, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else if ( 135.0 <= rotate && rotate < 225.0)
    {
      g.rotate( Math.PI * rotate/180, width/2, height/2);
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    }
    else if ( 225.0 <= rotate && rotate < 315.0)
    {
      g.translate( -height, 0.0);
      g.rotate( Math.PI * rotate/180, height, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    if ( (dd.dynType & GeDyn.mDynType_Rotate) != 0 && dd.rotate != 0) {
      g.rotate( Math.PI * dd.rotate/180, 

           (dd.x0 - getX())*original_width/width,
           (dd.y0 - getY()) * original_height / height);
    }
    AffineTransform save = g.getTransform();
    AffineTransform save_tmp;
    int rounds = 1;
    if ( fillLevel != 1F)
      rounds = 2;
    int oldColor = 0;
    for ( int i = 0; i < rounds; i++) {
      if ( rounds == 2) {
        switch ( i) {
        case 0:
	  if ( levelColorTone != GeColor.COLOR_TONE_NO) {
	    oldColor = colorTone;
	    colorTone = levelColorTone;
	  }
	  else if ( levelFillColor != GeColor.COLOR_NO) {
	    oldColor = fillColor;
	    fillColor = levelFillColor;
	  }
	  break;
        case 1:
	  if ( levelColorTone != GeColor.COLOR_TONE_NO)
	    colorTone = oldColor;
	  else if ( levelFillColor != GeColor.COLOR_NO)
	    fillColor = oldColor;
	  break;
        }
        switch ( levelDirection) {
        case Ge.DIRECTION_UP:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,fillLevel*original_height+Ge.cJBean_Offset,original_width,original_height));
	  else
	    g.setClip(new Rectangle2D.Float(0F,0F,original_width,fillLevel * original_height+Ge.cJBean_Offset));
	  break;
	case Ge.DIRECTION_DOWN:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,0F,original_width,(1-fillLevel)*original_height+Ge.cJBean_Offset));
	  else
	    g.setClip(new Rectangle2D.Float(0F,(1-fillLevel)*original_height+Ge.cJBean_Offset,original_width,original_height));
	  break;
	case Ge.DIRECTION_RIGHT:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(fillLevel*original_width+Ge.cJBean_Offset,0F,original_width,original_height));
	  else
	    g.setClip(new Rectangle2D.Float(0F,0F,fillLevel*width+Ge.cJBean_Offset,height));
	  break;
	case Ge.DIRECTION_LEFT:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,0F,(1-fillLevel)*original_width+Ge.cJBean_Offset,original_height));
	  else
	    g.setClip(new Rectangle2D.Float((1-fillLevel)*original_width+Ge.cJBean_Offset,0F,original_width,original_height));
	  break;
	}
      }
    {
    int fcolor = GeColor.getDrawtype(300, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    g.setColor(GeColor.getColor( fcolor));
    g.fill( shapes_p7[0]);
    }
    if ( shadow == 0) {
    ((Arc2D)shapes_p7[1]).setArcType(Arc2D.PIE);
    g.setColor(GeColor.getColor(74, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed));
    g.fill( shapes_p7[1]);
    ((Arc2D)shapes_p7[1]).setArcType(Arc2D.OPEN);
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes_p7[1]);
    } else {
    ((Arc2D)shapes_p7[2]).setArcType(Arc2D.PIE);
    ((Arc2D)shapes_p7[3]).setArcType(Arc2D.PIE);
    ((Arc2D)shapes_p7[4]).setArcType(Arc2D.PIE);
    ((Arc2D)shapes_p7[5]).setArcType(Arc2D.PIE);
    ((Arc2D)shapes_p7[6]).setArcType(Arc2D.PIE);
    {
    int fcolor = GeColor.getDrawtype(74, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, fillColor, dimmed);
    g.setColor(GeColor.shiftColor( fcolor, -2, colorInverse));
    g.fill( shapes_p7[2]);
    g.setColor(GeColor.shiftColor( fcolor, 2, colorInverse));
    g.fill( shapes_p7[3]);
    g.setColor(GeColor.getColor( fcolor));
    g.fill( shapes_p7[4]);
    g.fill( shapes_p7[5]);
    g.fill( shapes_p7[6]);
    }
    ((Arc2D)shapes_p7[7]).setArcType(Arc2D.OPEN);
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes_p7[7]);
    }
    g.setStroke( new BasicStroke(1F));
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, borderColor, dimmed));
    g.draw( shapes_p7[8]);
    }
    if ( rounds == 2)
      g.setClip(null);
    g.setTransform(save);
  }
  public Dimension getPreferredSize() { return size;}
  public Dimension getMinimumSize() { return size;}
}
protected class Grp166_ extends GeComponent {
  // Dimension size;
  public Grp166_( JopSession session)
  {
    super( session);
    size = new Dimension( 35, 18);
  }
  public int original_width = 35;
  public int original_height = 18;
  Shape[] shapes = new Shape[] { 
  };
  public void paintComponent(Graphics g1) {
    animationCount = 1;
    if ( !visible)
      return;
    Graphics2D g = (Graphics2D) g1;
    float width = getWidth();
    float height = getHeight();
    g.setRenderingHint(RenderingHints.KEY_ANTIALIASING,RenderingHints.VALUE_ANTIALIAS_ON);
    if ( 45.0 <= rotate && rotate < 135.0) {
      g.translate( width, 0.0); 
      g.rotate( Math.PI * rotate/180, 0.0, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else if ( 135.0 <= rotate && rotate < 225.0)
    {
      g.rotate( Math.PI * rotate/180, width/2, height/2);
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    }
    else if ( 225.0 <= rotate && rotate < 315.0)
    {
      g.translate( -height, 0.0);
      g.rotate( Math.PI * rotate/180, height, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    if ( (dd.dynType & GeDyn.mDynType_Rotate) != 0 && dd.rotate != 0) {
      g.rotate( Math.PI * dd.rotate/180, 

           (dd.x0 - getX())*original_width/width,
           (dd.y0 - getY()) * original_height / height);
    }
    AffineTransform save = g.getTransform();
    AffineTransform save_tmp;
    int rounds = 1;
    if ( fillLevel != 1F)
      rounds = 2;
    int oldColor = 0;
    for ( int i = 0; i < rounds; i++) {
      if ( rounds == 2) {
        switch ( i) {
        case 0:
	  if ( levelColorTone != GeColor.COLOR_TONE_NO) {
	    oldColor = colorTone;
	    colorTone = levelColorTone;
	  }
	  else if ( levelFillColor != GeColor.COLOR_NO) {
	    oldColor = fillColor;
	    fillColor = levelFillColor;
	  }
	  break;
        case 1:
	  if ( levelColorTone != GeColor.COLOR_TONE_NO)
	    colorTone = oldColor;
	  else if ( levelFillColor != GeColor.COLOR_NO)
	    fillColor = oldColor;
	  break;
        }
        switch ( levelDirection) {
        case Ge.DIRECTION_UP:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,fillLevel*original_height+Ge.cJBean_Offset,original_width,original_height));
	  else
	    g.setClip(new Rectangle2D.Float(0F,0F,original_width,fillLevel * original_height+Ge.cJBean_Offset));
	  break;
	case Ge.DIRECTION_DOWN:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,0F,original_width,(1-fillLevel)*original_height+Ge.cJBean_Offset));
	  else
	    g.setClip(new Rectangle2D.Float(0F,(1-fillLevel)*original_height+Ge.cJBean_Offset,original_width,original_height));
	  break;
	case Ge.DIRECTION_RIGHT:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(fillLevel*original_width+Ge.cJBean_Offset,0F,original_width,original_height));
	  else
	    g.setClip(new Rectangle2D.Float(0F,0F,fillLevel*width+Ge.cJBean_Offset,height));
	  break;
	case Ge.DIRECTION_LEFT:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,0F,(1-fillLevel)*original_width+Ge.cJBean_Offset,original_height));
	  else
	    g.setClip(new Rectangle2D.Float((1-fillLevel)*original_width+Ge.cJBean_Offset,0F,original_width,original_height));
	  break;
	}
      }
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, textColor, dimmed));
    g.setFont(new Font("Helvetica", Font.BOLD, 12));
    g.drawString( "Local",2, 13);
    }
    if ( rounds == 2)
      g.setClip(null);
    g.setTransform(save);
  }
  public Dimension getPreferredSize() { return size;}
  public Dimension getMinimumSize() { return size;}
}
protected class Grp173_ extends GeComponent {
  // Dimension size;
  public Grp173_( JopSession session)
  {
    super( session);
    size = new Dimension( 38, 17);
  }
  public int original_width = 38;
  public int original_height = 17;
  Shape[] shapes = new Shape[] { 
  };
  public void paintComponent(Graphics g1) {
    animationCount = 1;
    if ( !visible)
      return;
    Graphics2D g = (Graphics2D) g1;
    float width = getWidth();
    float height = getHeight();
    g.setRenderingHint(RenderingHints.KEY_ANTIALIASING,RenderingHints.VALUE_ANTIALIAS_ON);
    if ( 45.0 <= rotate && rotate < 135.0) {
      g.translate( width, 0.0); 
      g.rotate( Math.PI * rotate/180, 0.0, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else if ( 135.0 <= rotate && rotate < 225.0)
    {
      g.rotate( Math.PI * rotate/180, width/2, height/2);
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    }
    else if ( 225.0 <= rotate && rotate < 315.0)
    {
      g.translate( -height, 0.0);
      g.rotate( Math.PI * rotate/180, height, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    if ( (dd.dynType & GeDyn.mDynType_Rotate) != 0 && dd.rotate != 0) {
      g.rotate( Math.PI * dd.rotate/180, 

           (dd.x0 - getX())*original_width/width,
           (dd.y0 - getY()) * original_height / height);
    }
    AffineTransform save = g.getTransform();
    AffineTransform save_tmp;
    int rounds = 1;
    if ( fillLevel != 1F)
      rounds = 2;
    int oldColor = 0;
    for ( int i = 0; i < rounds; i++) {
      if ( rounds == 2) {
        switch ( i) {
        case 0:
	  if ( levelColorTone != GeColor.COLOR_TONE_NO) {
	    oldColor = colorTone;
	    colorTone = levelColorTone;
	  }
	  else if ( levelFillColor != GeColor.COLOR_NO) {
	    oldColor = fillColor;
	    fillColor = levelFillColor;
	  }
	  break;
        case 1:
	  if ( levelColorTone != GeColor.COLOR_TONE_NO)
	    colorTone = oldColor;
	  else if ( levelFillColor != GeColor.COLOR_NO)
	    fillColor = oldColor;
	  break;
        }
        switch ( levelDirection) {
        case Ge.DIRECTION_UP:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,fillLevel*original_height+Ge.cJBean_Offset,original_width,original_height));
	  else
	    g.setClip(new Rectangle2D.Float(0F,0F,original_width,fillLevel * original_height+Ge.cJBean_Offset));
	  break;
	case Ge.DIRECTION_DOWN:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,0F,original_width,(1-fillLevel)*original_height+Ge.cJBean_Offset));
	  else
	    g.setClip(new Rectangle2D.Float(0F,(1-fillLevel)*original_height+Ge.cJBean_Offset,original_width,original_height));
	  break;
	case Ge.DIRECTION_RIGHT:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(fillLevel*original_width+Ge.cJBean_Offset,0F,original_width,original_height));
	  else
	    g.setClip(new Rectangle2D.Float(0F,0F,fillLevel*width+Ge.cJBean_Offset,height));
	  break;
	case Ge.DIRECTION_LEFT:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,0F,(1-fillLevel)*original_width+Ge.cJBean_Offset,original_height));
	  else
	    g.setClip(new Rectangle2D.Float((1-fillLevel)*original_width+Ge.cJBean_Offset,0F,original_width,original_height));
	  break;
	}
      }
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, textColor, dimmed));
    g.setFont(new Font("Helvetica", Font.BOLD, 12));
    g.drawString( "Motor",2, 13);
    }
    if ( rounds == 2)
      g.setClip(null);
    g.setTransform(save);
  }
  public Dimension getPreferredSize() { return size;}
  public Dimension getMinimumSize() { return size;}
}
protected class Grp182_ extends GeComponent {
  // Dimension size;
  public Grp182_( JopSession session)
  {
    super( session);
    size = new Dimension( 43, 18);
  }
  public int original_width = 43;
  public int original_height = 18;
  Shape[] shapes = new Shape[] { 
  };
  public void paintComponent(Graphics g1) {
    animationCount = 1;
    if ( !visible)
      return;
    Graphics2D g = (Graphics2D) g1;
    float width = getWidth();
    float height = getHeight();
    g.setRenderingHint(RenderingHints.KEY_ANTIALIASING,RenderingHints.VALUE_ANTIALIAS_ON);
    if ( 45.0 <= rotate && rotate < 135.0) {
      g.translate( width, 0.0); 
      g.rotate( Math.PI * rotate/180, 0.0, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else if ( 135.0 <= rotate && rotate < 225.0)
    {
      g.rotate( Math.PI * rotate/180, width/2, height/2);
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    }
    else if ( 225.0 <= rotate && rotate < 315.0)
    {
      g.translate( -height, 0.0);
      g.rotate( Math.PI * rotate/180, height, 0.0);
      g.transform( AffineTransform.getScaleInstance( height/original_width,
      		width/original_height));
    }
    else
      g.transform( AffineTransform.getScaleInstance( width/original_width,
      		height/original_height));
    if ( (dd.dynType & GeDyn.mDynType_Rotate) != 0 && dd.rotate != 0) {
      g.rotate( Math.PI * dd.rotate/180, 

           (dd.x0 - getX())*original_width/width,
           (dd.y0 - getY()) * original_height / height);
    }
    AffineTransform save = g.getTransform();
    AffineTransform save_tmp;
    int rounds = 1;
    if ( fillLevel != 1F)
      rounds = 2;
    int oldColor = 0;
    for ( int i = 0; i < rounds; i++) {
      if ( rounds == 2) {
        switch ( i) {
        case 0:
	  if ( levelColorTone != GeColor.COLOR_TONE_NO) {
	    oldColor = colorTone;
	    colorTone = levelColorTone;
	  }
	  else if ( levelFillColor != GeColor.COLOR_NO) {
	    oldColor = fillColor;
	    fillColor = levelFillColor;
	  }
	  break;
        case 1:
	  if ( levelColorTone != GeColor.COLOR_TONE_NO)
	    colorTone = oldColor;
	  else if ( levelFillColor != GeColor.COLOR_NO)
	    fillColor = oldColor;
	  break;
        }
        switch ( levelDirection) {
        case Ge.DIRECTION_UP:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,fillLevel*original_height+Ge.cJBean_Offset,original_width,original_height));
	  else
	    g.setClip(new Rectangle2D.Float(0F,0F,original_width,fillLevel * original_height+Ge.cJBean_Offset));
	  break;
	case Ge.DIRECTION_DOWN:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,0F,original_width,(1-fillLevel)*original_height+Ge.cJBean_Offset));
	  else
	    g.setClip(new Rectangle2D.Float(0F,(1-fillLevel)*original_height+Ge.cJBean_Offset,original_width,original_height));
	  break;
	case Ge.DIRECTION_RIGHT:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(fillLevel*original_width+Ge.cJBean_Offset,0F,original_width,original_height));
	  else
	    g.setClip(new Rectangle2D.Float(0F,0F,fillLevel*width+Ge.cJBean_Offset,height));
	  break;
	case Ge.DIRECTION_LEFT:
	  if ( i == 0)
	    g.setClip(new Rectangle2D.Float(0F,0F,(1-fillLevel)*original_width+Ge.cJBean_Offset,original_height));
	  else
	    g.setClip(new Rectangle2D.Float((1-fillLevel)*original_width+Ge.cJBean_Offset,0F,original_width,original_height));
	  break;
	}
      }
    g.setColor(GeColor.getColor(0, colorTone,
	 colorShift, colorIntensity, colorBrightness, colorInverse, textColor, dimmed));
    g.setFont(new Font("Helvetica", Font.BOLD, 12));
    g.drawString( "Extern",2, 13);
    }
    if ( rounds == 2)
      g.setClip(null);
    g.setTransform(save);
  }
  public Dimension getPreferredSize() { return size;}
  public Dimension getMinimumSize() { return size;}
}
  public int getUtilityType() {
    return JopUtility.GRAPH;
  }
  public PwrtObjid getUtilityObjid() {
    return utilityAref.getObjid();
  }
  public PwrtAttrRef getUtilityAttrRef() {
    return utilityAref;
  }
  public String getUtilityName() {
    return this.getClass().getName();
  }
}
