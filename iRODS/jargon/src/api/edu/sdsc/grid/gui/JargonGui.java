//	Copyright (c) 2005, Regents of the University of California
//	All rights reserved.
//
//	Redistribution and use in source and binary forms, with or without
//	modification, are permitted provided that the following conditions are
//	met:
//
//	  * Redistributions of source code must retain the above copyright notice,
//	this list of conditions and the following disclaimer.
//	  * Redistributions in binary form must reproduce the above copyright
//	notice, this list of conditions and the following disclaimer in the
//	documentation and/or other materials provided with the distribution.
//	  * Neither the name of the University of California, San Diego (UCSD) nor
//	the names of its contributors may be used to endorse or promote products
//	derived from this software without specific prior written permission.
//
//	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
//	IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
//	THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
//	PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
//	CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
//	EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
//	PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
//	PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
//	LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
//	NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
//	SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//
//  FILE
//	JargonGui.java	-  edu.sdsc.grid.gui.JargonGui
//
//  CLASS HIERARCHY
//	javax.swing.JTree
//	    |
//	    +-edu.sdsc.grid.gui.JargonGui
//
//  PRINCIPAL AUTHOR
//	Lucas Gilbert, SDSC/UCSD
//
//
package edu.sdsc.grid.gui;



import java.io.IOException;
import java.util.HashMap;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyEvent;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.awt.event.MouseListener;

import javax.swing.JFrame;
import javax.swing.JMenuItem;
import javax.swing.JPanel;
import javax.swing.JPopupMenu;
import javax.swing.JTabbedPane;
import javax.swing.JToolBar;
import javax.swing.JTree;
import javax.swing.tree.TreePath;

import edu.sdsc.grid.io.FileFactory;
import edu.sdsc.grid.io.GeneralFile;
import edu.sdsc.grid.io.MetaDataSelect;
import edu.sdsc.grid.io.MetaDataSet;



//TODO
import edu.sdsc.grid.io.*;
import edu.sdsc.grid.io.srb.*;
import edu.sdsc.grid.io.local.*;

import java.awt.BorderLayout;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;


//TODO
import java.awt.*;
import java.net.*;
import javax.swing.*;
import javax.swing.table.*;
import javax.swing.tree.*;
import javax.swing.event.*;
/*
//TODO
import dgtab.*;
import dgtab.DGTab;
//for the matrix tab

//TODO
import edu.sdsc.SrbAdmin.*;
import edu.sdsc.SrbAdmin.mcatAdmin;
*/

/**
 *
 *
 * @author Lucas Ammon Gilbert
 * @since JARGON1.5
 */
/*TODO public*/ class JargonGui extends JFrame implements ActionListener
{
//----------------------------------------------------------------------
// Constants
//----------------------------------------------------------------------


//----------------------------------------------------------------------
// Fields
//----------------------------------------------------------------------
	/**
	 * Stores a selected abstract filepath.
	 * To be used in one tab or another.
	 */
	private JTextField addresField;

	/**
	 * File currently selected on the tab and stored in the address bar
	 */
	private GeneralFile selectedFile;

	/**
	 *
	 */
	private JTabbedPane tabPane;

	/**
	 *
	 */
	private JargonTab[] tabs;

	/**
	 *
	 */
	private JToolBar toolBar;



//TODO
	static HashMap registerTabs = new HashMap();
	static {
//TODO		registerTabs.add( "srbConnectionTab" );
		registerTabs.put( "browseTab", JargonTree.class );
		registerTabs.put( "transferTab", TransferStatusPanel.class );
		registerTabs.put( "queryTab", QueryPanel.class );
//TODO		registerTabs.add( "executeTab", ExecutePanel.class );
//TODO		registerTabs.add( "srbAdminTab", SRBAdmin.class );
	}


JComponent contentPane;
JScrollPane mainPanel;
JScrollPane sidePanel;
private GridBagConstraints gridBagConstraints;

private GeneralFileSystem[] fileSystems;


//----------------------------------------------------------------------
// Constructors & Destructors
//----------------------------------------------------------------------
	/**
	 *
	 */
	public JargonGui( )
		throws IOException
	{
		this( (GeneralFile)null );
	}

	/**
	 *
	 */
	public JargonGui( String configFilePath )
		throws IOException
	{
		this( new LocalFile( configFilePath ) );
	}

	/**
	 *
	 */
	public JargonGui( GeneralFile configFile )
		throws IOException
	{
		if ( configFile == null ) {
			useTabs( (GeneralFile)null );
		}
		else {
			useTabs( configFile );
		}

		init();
	}

	/**
	 *
	 */
	public JargonGui( JargonTab[] jargonTabs )
		throws IOException
	{
		if ( jargonTabs == null ) {
			useTabs( (GeneralFile)null );
		}
		else {
			useTabs( jargonTabs );
		}

		init();
	}



	/**
	 * Finalizes the object by explicitly letting go of each of
	 * its internally held values.
	 */
	protected void finalize()
		throws Throwable
	{
		if (addresField != null)
			addresField = null;
		if (selectedFile != null)
			selectedFile = null;
		if (tabPane != null)
			tabPane = null;
		if (tabs != null)
			tabs = null;
		if (toolBar != null)
			toolBar = null;
		if (registerTabs != null)
			registerTabs = null;

		super.finalize();
	}


//JButton submitButton;
JButton queryButton;
	private void init( )
		throws IOException
	{
	try {
UIManager.setLookAndFeel(
	    "com.sun.java.swing.plaf.windows.WindowsLookAndFeel");
} catch ( Throwable e ) {
//whatever
}
		//for adding a new recordList, make sure each is on its own line.
		gridBagConstraints = new GridBagConstraints();
		//Width = to the end of the line.

		contentPane = (JComponent) getContentPane();
//		contentPane.setLayout( new GridBagLayout() );

Box toolBarBox = new Box( BoxLayout.X_AXIS );
toolBarBox.add(Box.createGlue());

//TODO test
toolBar = new JToolBar();

ImageIcon buttonIcon = new ImageIcon("query.gif");

queryButton = new JButton(buttonIcon);
queryButton.setActionCommand("Query");
queryButton.setToolTipText("Query");
queryButton.setBackground(Color.WHITE);
		queryButton.setMargin(new java.awt.Insets(0, 0, 0, 0));
queryButton.addActionListener(this);

/*
buttonIcon = new ImageIcon("submit3.gif");
		submitButton = new JButton( buttonIcon );
submitButton.setActionCommand("Submit");
		submitButton.setToolTipText( QueryPanel.SUBMIT_BUTTON_TEXT );
		submitButton.setMargin(new java.awt.Insets(0, 0, 0, 0));
//TODO querytab gets special integration
((QueryPanel)queryTab.getComponent()).setSubmitButton( submitButton );
//	submitButton.addActionListener((QueryPanel)queryTab.getComponent());
submitButton.addActionListener(this);
*/

toolBar.add( queryButton );
toolBarBox.add( toolBar, BorderLayout.WEST );
//toolBar.add( submitButton );
//toolBar.add( new JButton("some button"));
//contentPane.add( toolBar, BorderLayout.WEST  );

		gridBagConstraints.gridwidth = GridBagConstraints.REMAINDER;

//textfield was being a pain.
//JPanel addressPanel = new JPanel();

JToolBar addressBar = new JToolBar();
//addressPanel.setPreferredSize( new Dimension( 600, 25 ) );
		addresField = new JTextField(70);
		addresField.addActionListener(new ActionListener() {
			public void actionPerformed(ActionEvent e)
			{
				String input = ((JTextField)e.getSource()).getText();

//forget all this, make a combobox wiith a list of filesystems
//and addressfield has just path
				URI uri = null;
				try {
					uri = new URI(input);
					selectedFile = FileFactory.newFile( uri );
				} catch (URISyntaxException ex) {
					JOptionPane.showMessageDialog((Component)e.getSource(),
						"Cannot find "+input+". Check the spelling and try again, or try "+
						"searching for the item by clicking the Query button and then "+
						"clicking Search.",
						"Address Bar",
						JOptionPane.ERROR_MESSAGE );
				} catch (SecurityException ex) {
					for (int i=0;i<fileSystems.length;i++) {
//						if (
//						newFile( uri, password )
					}
				}
				catch (SRBException ex) {
					JOptionPane.showMessageDialog((Component)e.getSource(),
						ex.getMessage()+"\n"+ex.getStandardMessage()+" "+ex.getType(),
						"Error",
						JOptionPane.ERROR_MESSAGE );
				}
				catch (IOException ex) {
					JOptionPane.showMessageDialog((Component)e.getSource(),
						ex.getMessage(),
						"Error",
						JOptionPane.ERROR_MESSAGE );
				}
			}
		});


		addresField.setPreferredSize( new Dimension( 500, 20 ) );
//		addressPanel.add( new JLabel("Address "), BorderLayout.EAST );
addressBar.add( new JLabel("Address ") );
//		addressPanel.add( addresField, BorderLayout.WEST );
addressBar.add( addresField );
//		contentPane.add( addressPanel, gridBagConstraints );

toolBarBox.add( addressBar, BorderLayout.EAST );

contentPane.add( toolBarBox, BorderLayout.NORTH );

setSidePanel( browseTab );
setMainPanel( queryTab );
Box contentBox = new Box( BoxLayout.X_AXIS );

contentBox.add( sidePanel, BorderLayout.WEST );
contentBox.add( mainPanel, BorderLayout.EAST );
contentPane.add( contentBox, BorderLayout.SOUTH );
	}

JargonTree jargonTree;
BrowseTab browseTab;
QueryTab queryTab;
//TODO got rid of the tabs idea
	private void useTabs( GeneralFile configFile )
		throws IOException
	{
try {
GeneralFile files[] = {
	FileFactory.newFile( new URI("file:///") ),
	FileFactory.newFile( new SRBFileSystem(srbAccount), "/" ),
	FileFactory.newFile( new SRBFileSystem(srbAccount), srbAccount.getHomeDirectory() ),
};
fileSystems = new GeneralFileSystem[files.length];
fileSystems[0] = files[0].getFileSystem();
fileSystems[1] = files[1].getFileSystem();
fileSystems[2] = files[2].getFileSystem();

jargonTree = new JargonTree( files );
jargonTree.setEditable(false);
jargonTree.useDefaultPopupMenu( true );
/*jargonTree.setCellEditor( new DefaultTreeCellEditor(jargonTree, new DefaultTreeCellRenderer()) {
	public Component getTreeCellEditorComponent( JTree tree, Object value,
		boolean isSelected, boolean expanded, boolean leaf, int row)
	{
		System.out.println(value);
		return new JLabel(((GeneralFile)value).getName());
	}
});*/
browseTab = new BrowseTab( jargonTree );

queryTab = new QueryTab( new QueryPanel( files[0] ) );

//TODO actually only create as needed
//transferTab = new TransferTab( new TransferStatusPanel( files ) );

//TODO
//executeTab = new ExecuteTab( new JargonTree( files ) );
//TODO
//adminTab = new AdminTab( new mcatAdmin( "" ) );
//TODO presumably
//matrixTab = new MatrixTab( new DGTab() );
} catch (Throwable e) { e.printStackTrace(); }


/*TODO
		int index = 0, nextIndex = -1;
		GeneralRandomAccessFile raf =
			FileFactory.newRandomAccessFile( configFile );
		String configData = new String( configBytes );
		Class tabClass = null;
		JargonTab tab = null;

		while (true) {
			configData = raf.readLine();
			if (!configData.startsWith("#") && (configData.length() > 1)) {
				index = configData.indexOf(" ");
				tabClass = properties.get(configData.subString( 0, index-1 ));

				//get options
				nextIndex = configData.indexOf( ",", index+1 );
				while ( nextIndex>=0 ) {
					configData.subString( index );
					index = nextIndex;
					nextIndex = configData.indexOf( ",", nextIndex );
				}

				tab = tabClass
			}
		}
*/

	}

	private void useTabs( JargonTab[] tabs )
		throws IOException
	{

	}



//----------------------------------------------------------------------
// Setters and getters
//----------------------------------------------------------------------
	/**
	 * Changes the addresField to <code>file</code>.
	 */
	/*TODO? public*/ void updateAddressBar( GeneralFile file )
	{
		selectedFile = file;

		//has to be URIs
		addresField.setText( selectedFile.toString() );
	}

	/**
	 * Returns the abstract pathname of the file selected file.
	 * in the current tab.
	 */
	/*TODO? public*/ GeneralFile getSelectedFile()
	{
		return selectedFile;
	}



	/**
	 * Returns the abstract pathname of the file selected file.
	 * in the current tab.
	 */
	private void setJToolBar( JToolBar toolBar )
	{
/*
		if (this.toolBar == null) {

		}
*/
		if (toolBar == null) {
			return;
		}

		//TODO add together main toolbar and sub toolbar
		contentPane.add( toolBar, gridBagConstraints );
	}


	/**
	 * Returns the abstract pathname of the file selected file.
	 * in the current tab.
	 */
	public void setJMenuBar( JMenuBar menuBar )
	{
		if (menuBar == null)
			return;

		//TODO add together main menuBar and sub menuBar
		super.setJMenuBar( menuBar );
	}


	/**
	 * Returns the abstract pathname of the file selected file.
	 * in the current tab.
	 */
	private void setSidePanel( JargonTab tab )
		throws IOException
	{
		if ( sidePanel == null ) {
			sidePanel = new JScrollPane( tab.getComponent() );
			sidePanel.setPreferredSize( new Dimension( 200, 490 ) );
		}
		else {
			sidePanel.setViewportView( tab.getComponent( null ) );
		}
	}


	/**
	 * Returns the abstract pathname of the file selected file.
	 * in the current tab.
	 */
	private void setMainPanel( JargonTab tab )
		throws IOException
	{
		if (mainPanel == null) {
			mainPanel = new JScrollPane( tab.getComponent() );
			mainPanel.setPreferredSize( new Dimension( 680, 490 ) );
		}
		else {
			GeneralFile[] files = { getSelectedFile() };
			mainPanel.setViewportView( tab.getComponent( files ) );
		}

//		setJToolBar( tab.getJToolBar() );
		setJMenuBar( tab.getJMenuBar() );
	}



//----------------------------------------------------------------------
// Example Methods
//----------------------------------------------------------------------
/*
	void useDefaultMenuBar( boolean useDefault )
	{
		if (!useDefault) {
			//remove popup menu
			menuBar = null;
			if (menuListener != null)
				this.removeMouseListener(menuListener);
			return;
		}

		menuBar = new JMenuBar();

		fileMenu = new JMenu( "File" );
			- new >
				- folder
				-
			- delete
			- rename
			- properties
			- refresh
			- exit

		editMenu
			- cut
			- copy
			- paste
			- replicate

		queryMenu
			- new query
			- query selected

		menuItem = new JMenuItem("A text-only menu item", KeyEvent.VK_T);

	}
*/

			String[] selectFieldNames = {
				SRBMetaDataSet.FILE_COMMENTS,
				SRBMetaDataSet.SIZE,
				SRBMetaDataSet.ACCESS_CONSTRAINT,
				SRBMetaDataSet.USER_NAME,
				SRBMetaDataSet.DEFINABLE_METADATA_FOR_FILES,
			};
			MetaDataSelect[] selects =	MetaDataSet.newSelection( selectFieldNames );

//----------------------------------------------------------------------
// Listeners and related
//----------------------------------------------------------------------
	/**
	 * Invoked when an action occurs.
	 */
	public void actionPerformed(ActionEvent e)
	{
/*
		if (submitButton.getActionCommand().equals(e.getActionCommand())) {
			try {
((QueryPanel)queryTab.getComponent()).submitQuery();
				mainPanel.setViewportView( ((QueryPanel)queryTab.getComponent()).getResultsPanel() );
mainPanel.setPreferredSize( new Dimension( 640, 490 ) );
			} catch (IOException ioe) {
				ioe.printStackTrace();
			}
		}
*/
		if (queryButton.getActionCommand().equals(e.getActionCommand())) {
			try {
				if (selectedFile != null)
					((QueryPanel) queryTab.getComponent()).setFileSystem( selectedFile.getFileSystem() );
//TODO conditions
				setMainPanel( queryTab );
			} catch (IOException ioe) {
				ioe.printStackTrace();
			}
		}
	}

static SRBAccount srbAccount;
static {
	try {
		srbAccount = new SRBAccount();
	} catch (Throwable e) {

	}
}
//----------------------------------------------------------------------
// Testing Methods
//----------------------------------------------------------------------
	/**
	 * Stand alone testing.
	 */
	public static void main( String[] args )
	{
		JargonGui jargonGui = null;
		try {
			if (args.length == 0) {
				jargonGui = new JargonGui();
			}
			else if (args[0].equals("-uri")) {
				//TODO have a root files option?
				jargonGui = new JargonGui();
			}
			else if (args.length == 7) {
				//host, port, user name, password,
				//home directory, mdas domain home, default storage resource
				srbAccount = new SRBAccount(
					args[0], Integer.parseInt( args[1] ), args[2], args[3],
					args[4], args[5], args[6] );
				jargonGui = new JargonGui();
			}
			else if (args.length == 8) {
				//host, port, user name, password,
				//home directory, mdas domain home, default storage resource,
				//certificate authority
				srbAccount = new SRBAccount(
					args[0], Integer.parseInt( args[1] ), args[2], args[3],
					args[4], args[5], args[6] );
				//If the CA locations are not defined in your cog.properties file:
				srbAccount.setCertificateAuthority( args[7] );
				jargonGui = new JargonGui();

			}

			jargonGui.addWindowListener(new WindowAdapter() {
				public void windowClosing(WindowEvent we) {
					System.exit(0);
				}
			});
			jargonGui.setTitle("DataGrid Explorer");
			jargonGui.pack();
			jargonGui.show();
			jargonGui.validate();
		} catch (Throwable e) {
			e.printStackTrace();
			System.out.println(((SRBException)e).getStandardMessage());
		}
	}
/*
more at http://utenti.lycos.it/yanorel6/2/ch15.htm

//also Working with Drag and Drop
http://utenti.lycos.it/yanorel6/2/ch16.htm

example using system clipboard
import java.awt.datatransfer.*;
import java.awt.*;

public class Java2Clipboard implements ClipboardOwner {
    public static void main(String[] args) throws Exception {
      Java2Clipboard jc = new Java2Clipboard();
      jc.toClipboard();
      Frame f = new Frame("Open a text editor and paste the message from Java");
      f.setSize(600,10);
      f.show();
    }

    public void toClipboard() {
      SecurityManager sm = System.getSecurityManager();
      if (sm != null) {
        try {
           sm.checkSystemClipboardAccess();
           }
        catch (Exception e) {e.printStackTrace();}
        }
      Toolkit tk = Toolkit.getDefaultToolkit();
      StringSelection st = new StringSelection("Hello world from Java");
      Clipboard cp = tk.getSystemClipboard();
      cp.setContents(st, this);
    }

    public void lostOwnership(Clipboard clip, Transferable tr) {
       System.out.println("Lost Clipboard Ownership?!?");
    }
}
*/
class BrowseTab implements JargonTab
{
	JargonTree display;

	public /*TODO static*/ String tabName = "Browse";

	public /*TODO static*/ int mnemonic = KeyEvent.VK_B;

	public BrowseTab( JargonTree display )
	{
		this.display = display;
		display.addTreeSelectionListener( new TreeSelectionListener() {
			public void valueChanged(TreeSelectionEvent e)
			{
				Object selected = e.getPath().getLastPathComponent();
				if ( selected instanceof String ) {
					//clicked on top level "DataGrid" folder
					queryButton.setEnabled(false);
				}
				else {
					selectedFile = (GeneralFile) selected;
					addresField.setText( selectedFile.toString() );
//TODO GeneralFile.queriable()?
					if (selectedFile instanceof LocalFile)
						queryButton.setEnabled(false);
					else
						queryButton.setEnabled(true);
				}
			}
		});
	}

	public Component getComponent()
		throws IOException
	{
		return display;
	}

	public Component getComponent( GeneralFile[] file )
		throws IOException
	{
		if (file == null)
			return display;

		display = new JargonTree( file );
		return display;
	}

	public JMenuBar getJMenuBar()
	{
		return null;
	}

	public JToolBar getJToolBar()
	{
		return null;
	}
//TODO not sure if I need this...
	public GeneralFile getSelectedFile()
	{
		return (GeneralFile)display.getLastSelectedPathComponent();
	}

	public String getTabName()
	{
		return tabName;
	}

	public int getMnemonic()
	{
		return mnemonic;
	}
}
//TODO where to put these things -- end of JargonGui class
}
class TransferTab implements JargonTab
{
	TransferStatusPanel display;

	public static String tabName = "Transfer";

	public static int mnemonic = KeyEvent.VK_T;

	public TransferTab( TransferStatusPanel display )
	{
		this.display = display;
	}

	public Component getComponent()
		throws IOException
	{
		return display;
	}

	public Component getComponent( GeneralFile[] file )
		throws IOException
	{
		if (file == null)
			return display;

		display = new TransferStatusPanel( file );
		return display;
	}

	public JMenuBar getJMenuBar()
	{
		return null;
	}

	public JToolBar getJToolBar()
	{
		return null;
	}

	public GeneralFile getSelectedFile()
	{
		return null;
	}

	public String getTabName()
	{
		return tabName;
	}

	public int getMnemonic()
	{
		return mnemonic;
	}
}
class QueryTab implements JargonTab
{
	QueryPanel display;


	public static String tabName = "Query";

	public static int mnemonic = KeyEvent.VK_Q;

	public QueryTab( QueryPanel display )
	{
		this.display = display;
	}

	public Component getComponent()
		throws IOException
	{
		return display;
	}

	public Component getComponent( GeneralFile[] file )
		throws IOException
	{
		if (file == null)
			return display;

		display = new QueryPanel( file[0] );
		return display;
	}

	public JMenuBar getJMenuBar()
	{
		return null;
	}

	public JToolBar getJToolBar()
	{
		return null;
	}

	public GeneralFile getSelectedFile()
	{
		return null;
	}

	public String getTabName()
	{
		return tabName;
	}

	public int getMnemonic()
	{
		return mnemonic;
	}
}
class ExecuteTab implements JargonTab
{
//TODO ExecutePanel
	JargonTree display;


	public static String tabName = "Execute";

	public static int mnemonic = KeyEvent.VK_E;

	public ExecuteTab( JargonTree display )
	{
		this.display = display;
	}

	public Component getComponent()
		throws IOException
	{
		return display;
	}

	public Component getComponent( GeneralFile[] file )
		throws IOException
	{
		if (file == null)
			return display;

//TODO		display = new JargonTree( file );
		return display;
	}

	public JMenuBar getJMenuBar()
	{
		return null;
	}

	public JToolBar getJToolBar()
	{
		return null;
	}

	public GeneralFile getSelectedFile()
	{
		return null;
	}

	public String getTabName()
	{
		return tabName;
	}

	public int getMnemonic()
	{
		return mnemonic;
	}
}
class AdminTab implements JargonTab
{
//TODO SRBAdmin
//	mcatAdmin display;
	JargonTree display;

	public static String tabName = "Admin";

	public static int mnemonic = KeyEvent.VK_A;

	public AdminTab( JargonTree display )
	{
		this.display = display;
	}

	public Component getComponent()
		throws IOException
	{
		return display;
	}

	public Component getComponent( GeneralFile[] file )
		throws IOException
	{
		if (file == null)
			return display;

//TODO probably best just to ignore?		display = new mcatAdmin( file );
		return display;
	}

	public JMenuBar getJMenuBar()
	{
//		return display.getJMenuBar();
return null;
	}

	public JToolBar getJToolBar()
	{
		return null;
	}

	public GeneralFile getSelectedFile()
	{
		return null;
	}

	public String getTabName()
	{
		return tabName;
	}

	public int getMnemonic()
	{
		return mnemonic;
	}
}
class MatrixTab implements JargonTab
{
//	DGTab display;
	JargonTree display;


	public static String tabName = "Matrix";

	public static int mnemonic = KeyEvent.VK_M;

	public MatrixTab( JargonTree display )
	{
		this.display = display;
	}

	public Component getComponent()
		throws IOException
	{
		return display;
	}

	public Component getComponent( GeneralFile[] file )
		throws IOException
	{
		if (file == null)
			return display;

//TODO try to open as .dgl?		display = new DGTab( file );
		return display;
	}

	public JMenuBar getJMenuBar()
	{
		return null;
	}

	public JToolBar getJToolBar()
	{
//		return display.createToolBar();
return null;
	}

	public GeneralFile getSelectedFile()
	{
		return null;
	}

	public String getTabName()
	{
		return tabName;
	}

	public int getMnemonic()
	{
		return mnemonic;
	}
}
//maybe should not be a tab?
class ResultsTab implements JargonTab
{
	MetaDataDisplay display;


	public static String tabName = "Results";

	public static int mnemonic = KeyEvent.VK_R;

	public ResultsTab( MetaDataDisplay display )
	{
		this.display = display;
	}

	public Component getComponent()
		throws IOException
	{
		return display;
	}

	public Component getComponent( GeneralFile[] file )
		throws IOException
	{
		//TODO ? have to ignore?, needs MetaDataRecordList
		return display;
	}

	public JMenuBar getJMenuBar()
	{
		return null;
	}

	public JToolBar getJToolBar()
	{
		return null;
	}

	public GeneralFile getSelectedFile()
	{
		return null;
	}

	public String getTabName()
	{
		return tabName;
	}

	public int getMnemonic()
	{
		return mnemonic;
	}
}/*
class JargonGuiAction extends AbstractAction
{
	Action action;
	public JargonGuiAction(String name, Icon icon, Action action)
	{
		super(name, icon);

		this.action = action;
	}


	public void actionPerformed(ActionEvent evt)
	{
		if (listeners != null) {
			Object[] listenerList = listeners.getListenerList();

			// Recreate the ActionEvent and stuff the value of the
			// ACTION_COMMAND_KEY.
			ActionEvent e = new ActionEvent(evt.getSource(), evt.getID(),
				(String)getValue(Action.ACTION_COMMAND_KEY));

			for (int i = 0; i <= listenerList.length-2; i += 2) {
				((ActionListener)listenerList[i+1]).actionPerformed(e);
			}
		}
	}
}
	public void actionPerformed(ActionEvent evt)
	{
		String command = evt.getActionCommand();

		// Compare the action command to the known actions.
		if (command.equals(aboutAction.getActionCommand()))  {
			// The about action was invoked
			JOptionPane.showMessageDialog(this,
			aboutAction.getLongDescription(),
			aboutAction.getShortDescription(),
			JOptionPane.INFORMATION_MESSAGE);
		}
		else if (command.equals(cutAction.getActionCommand())) {
			copyBuffer = getSelectedFile();
			deleteOnPaste = true;
		}
		else if (command.equals(copyAction.getActionCommand())) {
			copyBuffer = getSelectedFile();
		}
		else if (command.equals(pasteAction.getActionCommand())) {
			copyBuffer.copyTo(getSelectedFile());
			if (deleteOnPaste)
				copyBuffer.delete();
		}
		else {
			getMainPanel().actionPerformed(evt);
		}
	}



Action[] currentActions;
*/
