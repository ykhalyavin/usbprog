/*
 * demo.java
 * Simple demo programm that shows
 * how to access usbprog with java,
 * using '_simpleport.so' wrapped
 * with SWIG.
 *
 * author: Michael Kaiser
 */

class example
{
  public static void main(String[] args){
    try {
      // tell the system to load the shared library into memory
      System.load("/home/bene/projects/usbprog/trunk/simpleport/lib/simpleport.so");
      //System.loadLibrary("../lib/simpleport");

      // the functions of '_simpleport.so' are accessed over the java-class
      // 'simpleport', that was created by SWIG.

      // 'simpleport_open()' returns a instance of 'SWIGTYPE_p_simpleport' if
      // a suitable hardware was found.

      SWIGTYPE_p_simpleport sp_handle = simpleport.simpleport_open();

      // set the port-direction to 'write'
      simpleport.simpleport_set_direction(sp_handle, (short) 0xFF);
      System.out.println("... blink!");

      // periodically set entire port to '00000000' and '11111111'

      while(true){
	simpleport.simpleport_set_port(sp_handle,(short) 0xFF,(short) 0xFF);
	Thread.sleep(1000);
	simpleport.simpleport_set_port(sp_handle, (short) 0x00, (short) 0xFF);
	Thread.sleep(1000);
      }
    } catch (Exception e) {
      e.toString();
    }
  }
}

