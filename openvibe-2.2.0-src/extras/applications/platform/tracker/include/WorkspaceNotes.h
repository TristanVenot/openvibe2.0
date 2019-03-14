
#pragma once

#include <openvibe/ov_all.h>

#include <fs/Files.h>

namespace OpenViBETracker 
{

/**
 * \class WorkspaceNotes 
 * \brief An overly complicated wrapper for a text string.
 * \author J. T. Lindgren
 *
 * This class is needed in order to use CCommentEditorDialog from Designer. The
 * implementation is a quick hack. IDs and Attributable features are not supported at all.
 *
 */
class WorkspaceNotes : public OpenViBE::Kernel::IComment
{
public:
	virtual OpenViBE::CIdentifier getIdentifier(void) const override { return 0; }

	virtual OpenViBE::CString getText(void) const override { return m_Notes; }

	virtual OpenViBE::boolean setIdentifier(
		const OpenViBE::CIdentifier& rIdentifier) override { return true; }

	virtual OpenViBE::boolean setText(
		const OpenViBE::CString& sText) { m_Notes = sText; return true; }

	virtual OpenViBE::boolean initializeFromExistingComment(
		const OpenViBE::Kernel::IComment& rExisitingComment) override {
		m_Notes = rExisitingComment.getText(); 
		return true;
	}

	// Attributable
	virtual OpenViBE::boolean addAttribute(
		const OpenViBE::CIdentifier& rAttributeIdentifier,
		const OpenViBE::CString& sAttributeValue)  {
		return true;
	}

	virtual OpenViBE::boolean removeAttribute(
		const OpenViBE::CIdentifier& rAttributeIdentifier) {
		return true;
	}
	virtual OpenViBE::boolean removeAllAttributes(void) { return true; }

	virtual OpenViBE::CString getAttributeValue(
		const OpenViBE::CIdentifier& rAttributeIdentifier) const {
		return OpenViBE::CString("");
	}
	virtual OpenViBE::boolean setAttributeValue(
		const OpenViBE::CIdentifier& rAttributeIdentifier,
		const OpenViBE::CString& sAttributeValue) {
		return true;
	}

	virtual OpenViBE::boolean hasAttribute(
		const OpenViBE::CIdentifier& rAttributeIdentifier) const {
		return false;
	}

	virtual OpenViBE::boolean hasAttributes(void) const { return false; }

	virtual OpenViBE::CIdentifier getNextAttributeIdentifier(
		const OpenViBE::CIdentifier& rPreviousIdentifier) const {
		return OpenViBE::CIdentifier();
	}

	OpenViBE::CIdentifier getClassIdentifier(void) const override { return OpenViBE::CIdentifier(); }

	// Methods specific to this derived class

	// Save the notes
	bool save(const OpenViBE::CString& notesFile)
	{
		std::ofstream output;
		FS::Files::openOFStream(output, notesFile.toASCIIString());
		if (!output.is_open() || output.bad()) 
		{
			return false;
		}
		output << m_Notes.toASCIIString();
		
		output.close();

		return true;
	}

	// Load the notes
	bool load(const OpenViBE::CString& notesFile)
	{
		if (FS::Files::fileExists(notesFile.toASCIIString()))
		{
			std::ifstream input;
			FS::Files::openIFStream(input, notesFile.toASCIIString());
			if (input.is_open() && !input.bad())
			{
				std::string str((std::istreambuf_iterator<char>(input)),
					std::istreambuf_iterator<char>());
				m_Notes = OpenViBE::CString(str.c_str());
				input.close();
				return true;
			}
		}

		return false;
	}

protected:

	OpenViBE::CString m_Notes;
};


};

