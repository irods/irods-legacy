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
//	JargonTabs.java	-  edu.sdsc.grid.gui.JargonTabs
//
//  CLASS HIERARCHY
//	java.awt.Component
//	    |
//	    +-edu.sdsc.grid.gui.JargonTabs
//
//  PRINCIPAL AUTHOR
//	Lucas Gilbert, SDSC/UCSD
//
//
package edu.sdsc.grid.gui;

import edu.sdsc.grid.io.*;

import java.awt.*;
import java.awt.event.*;
import java.io.*;
import javax.swing.*;

//TODO awt.Component interface
//which I can't do, but I don't want a subclass really...
//I guess, of, um, JPanel? no, just awt.Component, sigh.
/*TODO public? probably*/interface JargonTab {
	public Component getComponent() throws IOException;

	public Component getComponent(GeneralFile[] file) throws IOException;

	public JMenuBar getJMenuBar();

	// public JToolBar getJToolBar( );

	// public Action[] getActions( );

	// TODO not sure if I need this...
	public GeneralFile getSelectedFile();

	public String getTabName();

	public int getMnemonic();
}
