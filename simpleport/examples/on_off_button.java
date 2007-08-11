/**
 * HelloXIO.java
 *
 * Simple example that shows how to
 * access usbprog with java, using
 * '_simpleport.so' wrapped by SWIG.
 *
 * A simple GUI is provided.
 *
 * @author: Michael Kaiser
 *
 */

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.JButton;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JTextField;

public class HelloXIO extends JFrame{

	private static final long serialVersionUID = 1L;
	
	private JLabel lbText = new JLabel("Hello usbprog",0);
	private JButton btOn = new JButton("On");
	private JButton btOff = new JButton("Off");
	private JPanel plButtons = new JPanel();
   
	private SWIGTYPE_p_simpleport sp_handle;
    
	HelloXIO() {
        	super("simpleport");
        	plButtons.add(btOn);
        	plButtons.add(btOff);
        	getContentPane().add("North", lbText );
        	getContentPane().add("South", plButtons );
        	setSize(50,100);
        	pack();

        	btOn.addActionListener(new ActionListener(){
					public void actionPerformed(ActionEvent e){
						//simpleport.simpleport_set_port(sp_handle,(short) 0xFF,(short) 0xFF);
					}
        	});
        	
        	btOff.addActionListener(new ActionListener(){
					public void actionPerformed(ActionEvent e){
						//simpleport.simpleport_set_port(sp_handle,(short) 0x00,(short) 0xFF);
					}
        	});

        	setVisible(true);
        	setDefaultCloseOperation(EXIT_ON_CLOSE);
    	}
	
	public static void main(String[] argv) {
		HelloXIO inst = new HelloXIO();
		inst.connectToDevice();
	}

	private void connectToDevice() {
		System.load("_simpleport.so");
		sp_handle = simpleport.simpleport_open();
		simpleport.simpleport_set_direction(sp_handle, (short) 0xFF);
	}

}

