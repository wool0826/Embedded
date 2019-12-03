package ac.kr.kgu.esproject;

import java.util.Random;

import android.app.Activity;
import android.content.Context;
import android.os.Bundle;
import android.view.KeyEvent;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;

public class ArrayAdderActivity extends Activity {
	static { 
		System.loadLibrary("esterm");
	}
    
	public native int buzzerControl(int value);
	
	public native int segmentControl(int value);
	public native int segmentIOControl(int value);
	
	public native int dotMatrixControl(String data);
	public native int calculate(int[] arr, int value);
	// Thread Status
	
	private static final int IDLE = 1;
	private static final int PRINT = 2;
	private static final int INTERRUPT = 0;
	private int threadState = IDLE;
	
	private boolean start = false, restart = false;
	private boolean isConfirmed = false, isCorrect = false;
	
	private String value = "";
	
	BuzzThread buzzThread = new BuzzThread();
	DotThread dotThread = new DotThread();
	SegThread segThread = new SegThread();	
	
	// UI
	Button createBtn, confirmBtn, clearBtn;
	TextView elementList, resultTv;
	EditText inputValue;
	Spinner spinner;
	
	// Adder
	int[] arguments;
	Context context;
	
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
        
        dotThread.setDaemon(true);
        dotThread.start();
        segThread.setDaemon(true);
        segThread.start();
        //buzzThread.setDaemon(true);
        //buzzThread.start();
        
        // Initialize DotMatrix
        dotMatrixControl("00000000000000000000");
        
       // UI       
        createBtn = (Button) findViewById(R.id.createBtn);
        confirmBtn = (Button) findViewById(R.id.confirmBtn);
        clearBtn = (Button) findViewById(R.id.clearBtn);
        elementList = (TextView) findViewById(R.id.elementList);
        resultTv = (TextView) findViewById(R.id.resultTv);
        inputValue = (EditText) findViewById(R.id.inputValue);
        spinner = (Spinner) findViewById(R.id.spinner1);
        ArrayAdapter<CharSequence> adapter = ArrayAdapter.createFromResource(this,
             R.array.number, android.R.layout.simple_spinner_item);
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        spinner.setAdapter(adapter);

        createBtn.setOnClickListener(new Button.OnClickListener() {
			@Override
			public void onClick(View v) {
				Random r = new Random();
				StringBuilder str = new StringBuilder();
				
				int nums = spinner.getSelectedItemPosition() + 1;
				arguments = new int[nums];
				
				for(int i=1; i<=nums; i++){
					arguments[i-1] = r.nextInt(10);
					str.append("element #" + i + " : " + arguments[i-1] + "\n");
				}
				
				elementList.setText(str.toString());				
			}
		});
        
        confirmBtn.setOnClickListener(new Button.OnClickListener() {
			@Override
			public void onClick(View v) {
				value = inputValue.getText().toString();
							
				int argValue = Integer.parseInt(value);
				int result = calculate(arguments, argValue);
				
				if(result==argValue) isCorrect = true;
				else isCorrect = false;
				
				if(!isCorrect) value += "-" + result;
				
				isConfirmed = true;
				
				if (start) {
					restart = true;
				} else {
					start = true;
				}

				threadState = PRINT;
			}
		});
        
        clearBtn.setOnClickListener(new View.OnClickListener() {
			@Override
			public void onClick(View v) {
				// TODO Auto-generated method stub
				threadState = IDLE;
				
				start = false;
				restart = false;
				
				isConfirmed = false;
				
				int data = 0;
				buzzerControl(data);
			}
		});
    }
    
	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		if (keyCode == KeyEvent.KEYCODE_BACK) {
			threadState = INTERRUPT;
			buzzThread.interrupt();
			
			start = false;
			restart = false;
			dotThread.interrupt();
		}
		return super.onKeyDown(keyCode, event);
	}
	
    class BuzzThread extends Thread{
    	public void run(){
    		while(threadState != INTERRUPT){
    			int data = 0;
    			if(!isConfirmed){
    				data = 0;
    				buzzerControl(data);
    			} else {
    				if(isCorrect){
    					data = 0;
    					while(isConfirmed){
    						try {
    							buzzerControl(data);
        						data = (data == 0) ? 1 : 0;
								sleep(500);
							} catch (InterruptedException e) {
								// TODO Auto-generated catch block
								e.printStackTrace();
							}
    					}
    				} else {
    					data = 1;
    					buzzerControl(data);
    				}
    			}
    		}
    	}
    }
    class DotThread extends Thread{
    	public void run(){
    		while(threadState != INTERRUPT){
	    		if(!start){
	    	    	// idle..
	        	} else {
	        		if (value.length() > 50) {
	        			start = false;
	        			restart = false;
	        		} else {
	        			
	        			restart = false;
	        			int i, j, ch;
	        			char buf[] = new char[100];
	
	        			
	        			buf = value.toCharArray();
	
	        			String result = new String();
	        			
	        			result = "00000000000000000000";
	
	        			for (i = 0; i < value.length(); i++) {
	        			
	        				ch = Integer.valueOf(buf[i]);
	
	        			
	        				if (ch < 32 || ch > 126) {
	        					start = false;
	        					restart = false;
	        					break;
	        				}
	        			
	        				ch -= 0x20;
	
	        				// copy
	        				for (j = 0; j < 5; j++) {
	        					String str = new String();
	        					
	        			
	        					str = Integer.toHexString((Constants.font[ch][j]));
	        					
	        			
	        					if (str.length() < 2)
	        						result += "0";        					
	        			
	        					result += str;
	        				}
	        			
	        				result += "00";
	        			}
	        			
	        			result += "00000000000000000000";
	        	
	        			
	        			//
	        			
	        			// print
	        			for (i = 0; i < (result.length() - 18) / 2; i++) {
	        			
	        				for (j = 0; j < 15; j++) {
	        			
	        					if (!start) {
	        						break; 
	        					} else if (restart) {
	        						break; 
	        					} else {
	        						dotMatrixControl(result.substring(2 * i,
	        								2 * i + 20));
	        					}
	        				}
	        			}
	        			
	        			//
	        		}
	        		
	        		if (!restart)
	        			start = false;
	        		
	        		dotMatrixControl("00000000000000000000");
	        	}
    		}
    	}
    }
    class SegThread extends Thread{
    	public void run(){
    		while(threadState != INTERRUPT){
    			if(threadState == IDLE){
    				try{
	    				segmentIOControl(threadState);
	    				for(int i=0; i<16 && threadState == IDLE; i++){
	    					for(int j=0; j<10 && threadState == IDLE; j++){
	    						segmentControl(i);
	    					}
						}
    				} catch(Exception e){
    					
    				}    				
    			} else {
					segmentIOControl(threadState);
					
					for(int j=0; j<10 && threadState == PRINT; j++){
						segmentControl(2);
					}
    			}
    		}
    	}
    }
}