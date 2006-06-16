/* 
 * Proview   $Id: RatioLayout.java,v 1.5 2006-06-16 05:09:38 claes Exp $
 * Copyright (C) 2005 SSAB Oxel�sund AB.
 *
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License as 
 * published by the Free Software Foundation, either version 2 of 
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the 
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with the program, if not, write to the Free Software 
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

package jpwr.jop;
import java.awt.*;
import javax.swing.*;
import java.util.*;
import jpwr.jop.*;
import java.beans.Beans;

/** RatioLayout.java -- Layout manager for Java containers
 * 
 *  This layout manager allows you to specify ratios of x,y,width,height
 *  using a Proportion object.
 *
 *   For example,
 *
 *      setLayout(new RatioLayout());
 *      add( myObject, new Proportion(myObject.getBounds(), this.getSize()));
 *
 * @author Mats Trovik inspired by RatioLayout.java by Terence Parr 
 *
 */
public class RatioLayout implements LayoutManager2 {
    
    // track the ratios for each object of form "xratio,yratio;wratio,hratio"
    //Vector<Proportion> ratios = new Vector<Proportion>(1);
    Vector ratios = new Vector(1);
    // track the components also so we can remove associated modifier
    // if necessary.
    //Vector<Component> components = new Vector<Component>(1);
    Vector components = new Vector(1);
    
    public void addLayoutComponent(String r, Component comp) {
    }
    
    // The used method for adding components.
    public void addLayoutComponent(Component comp, Object o){
	Proportion r = (Proportion) o;
        ratios.addElement(r);
        components.addElement(comp);
    }
    //Maximum layout size depends on the target (as opposed to the normal case)
    public Dimension maximumLayoutSize(Container target){

	return  target.getSize();

    }
    
    public float getLayoutAlignmentX(Container target){
	//The layout is left aligned
	return (float) 0.0;
    }
 
    public float getLayoutAlignmentY(Container target){
	//the layout is top aligned
	return (float) 0.0;
    }
    //Reset the Layout
    public void invalidateLayout(Container target){
	 //ratios = new Vector<Proportion>(1);
	 //components = new Vector<Component>(1);
	 ratios = new Vector(1);
	 components = new Vector(1);
    }

    
    // Remove component from Layout
    public void removeLayoutComponent(Component comp) {
        int i = components.indexOf(comp);
        if ( i!=-1 ) {
            ratios.removeElementAt(i);
            components.removeElementAt(i);
        }
    }

    public Dimension preferredLayoutSize(Container target) {
        return target.getSize();
    }

    public Dimension minimumLayoutSize(Container target) {
        return target.getSize();
    }

    public void layoutContainer(Container target) {
	

	Insets insets = target.getInsets();
        int ncomponents = target.getComponentCount();
        Dimension d = target.getSize();
	//make sure to not draw over the insets.(Java standard)
	d.width -=  insets.left+insets.right;
        d.height -= insets.top+insets.bottom;
	//Layout each component
        for (int i = 0 ; i < ncomponents ; i++) {
            Component comp = target.getComponent(i);
            Proportion compProp;
	    try {
		compProp = (Proportion)ratios.elementAt(i);
	    } 
	    catch (ArrayIndexOutOfBoundsException e){
		break;
	    }
	    //Set the width and height according to the ratio specified when
	    //the component was added.
	    int w = (int)(d.width*compProp.rw);
	    int h = (int)(d.height*compProp.rh);
	    // set x & y to the ratio specified when the component was added.
	    //These values will be changed if the comonent is a slider or
	    //a moving GeComponent.
	    int x = (int) (d.width*compProp.rx);
	    int y = (int) (d.height*compProp.ry);	
	    if(comp instanceof GeComponent){
		GeComponent tempComp= (GeComponent) comp;
		//If the component is of type mDynType_Move...
		if((tempComp.dd.dynType & GeDyn.mDynType_Move) != 0){

		    if (compProp.firstDraw){
		    // ... and it's drawn for the first time, store the 
		    //parent's dimension in Proportion.previous,
			compProp.firstDraw=false;
			compProp.previous.setSize(d.width,d.height);
		    }


		    else{
		    /*... and it's drawn for at least the 2nd time, the
		     *place where to draw it is caculated from the ratio
		     *between the the current dimensions of the parent and
		     *the previous dimensions (before resize) 
		     *of the parent multiplied with the position of the 
		     *object. Care is taken to only adjust the position in
		     *the direction(s) specified by the GeDynMove.moveX(Y)Attribute.
		     *The current dimensions of the parent are stored
		     *in Proportion.previous.*/
			for(int j=0;j<tempComp.dd.elements.length;j++)
			    {
				if(tempComp.dd.elements[j] instanceof GeDynMove){
				    Rectangle compLoc = comp.getBounds();
				    if ((! ((GeDynMove)tempComp.dd.elements[j]).moveXAttribute.equals(""))){
					double rx = (1.0*d.width /compProp.previous.width );
					x = (int) (compLoc.x*rx);
				    } 
				    if ((! ((GeDynMove)tempComp.dd.elements[j]).moveYAttribute.equals(""))){
					double ry = (1.0*d.height/compProp.previous.height);
					y = (int) (compLoc.y*ry);
				    }
				    
				}
				compProp.previous.setSize(d.width,d.height);
			    }
		    }
		}

		//If the component is a slider...
		else if((tempComp.dd.actionType & GeDyn.mActionType_Slider) != 0){

		    if (compProp.firstDraw){
		    // ... and it's drawn for the first time, store the 
		    //parent's dimension in Proportion.previous,
			compProp.firstDraw=false;
			compProp.previous.setSize(d.width,d.height);
		    }


		    else{
		    /*... and it's drawn for at least the 2nd time, 
		     *the direction of the slider is checked.
		     *The ratio between the target width or height 
		     *(after resize) and the previous width or 
		     *height is used to calculate the position 
		     *fitting the new size. The current dimensions 
		     *of the parent are stored in Proportion.previous.*/
			for(int j=0;j<tempComp.dd.elements.length;j++)
			    {
				if(tempComp.dd.elements[j] instanceof GeDynSlider){
				    Rectangle compLoc = comp.getBounds();
				    if (
					((GeDynSlider)tempComp.dd.elements[j]).direction == Ge.DIRECTION_LEFT
					||
					((GeDynSlider)tempComp.dd.elements[j]).direction == Ge.DIRECTION_RIGHT){
					double rx = (1.0*d.width /compProp.previous.width );
					x = (int) (compLoc.x*rx);
				    } 
				    else if (
					((GeDynSlider)tempComp.dd.elements[j]).direction == Ge.DIRECTION_UP
					||
					((GeDynSlider)tempComp.dd.elements[j]).direction == Ge.DIRECTION_DOWN){
					double ry = (1.0*d.height/compProp.previous.height);
					y = (int) (compLoc.y*ry);
				    }
				    
				}
			    }	 
			compProp.previous.setSize(d.width,d.height);
		    }
		}
	    } 
	    //Place the component.
            comp.setBounds(x+insets.left,y+insets.top,w,h);

	    //Resize the font of JTextFields.
	    if (comp instanceof JTextField){
		comp.setFont(comp.getFont().deriveFont((float)(1.0*h*9/10)));

	       
	    }
        }
    }
    
    public String toString() {
        return getClass().getName();
    }
}













