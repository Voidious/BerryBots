#!/usr/bin/perl
##############################################################################
# FormMail                        Version 1.92                               #
# Copyright 1995-2002 Matt Wright mattw@scriptarchive.com                    #
# Created 06/09/95                Last Modified 04/21/02                     #
# Matt's Script Archive, Inc.:    http://www.scriptarchive.com/              #
##############################################################################
# COPYRIGHT NOTICE                                                           #
# Copyright 1995-2002 Matthew M. Wright  All Rights Reserved.                #
#                                                                            #
# FormMail may be used and modified free of charge by anyone so long as this #
# copyright notice and the comments above remain intact.  By using this      #
# code you agree to indemnify Matthew M. Wright from any liability that      #
# might arise from its use.                                                  #
#                                                                            #
# Selling the code for this program without prior written consent is         #
# expressly forbidden.  In other words, please ask first before you try and  #
# make money off of my program.                                              #
#                                                                            #
# Obtain permission before redistributing this software over the Internet or #
# in any other medium. In all cases copyright and header must remain intact. #
##############################################################################
# ACCESS CONTROL FIX: Peter D. Thompson Yezek                                #
#                     http://www.securityfocus.com/archive/1/62033           #
##############################################################################
# Define Variables                                                           #
#      Detailed Information Found In README File.                            #

# $mailprog defines the location of your sendmail program on your unix       #
# system. The flags -i and -t should be passed to sendmail in order to       #
# have it ignore single dots on a line and to read message for recipients    #

$mailprog = '/usr/sbin/sendmail -i -t';

# @referers allows forms to be located only on servers which are defined     #
# in this field.  This security fix from the last version which allowed      #
# anyone on any server to use your FormMail script on their web site.        #

@referers = ('184.72.231.183', 'playberrybots.com');

# @recipients defines the e-mail addresses or domain names that e-mail can   #
# be sent to.  This must be filled in correctly to prevent SPAM and allow    #
# valid addresses to receive e-mail.  Read the documentation to find out how #
# this variable works!!!  It is EXTREMELY IMPORTANT.                         #
@recipients = &fill_recipients(@referers);

### make this static, PJC 7/19/04
$recipient = 'voidious@gmail.com';

# ACCESS CONTROL FIX: Peter D. Thompson Yezek                                #
# @valid_ENV allows the sysadmin to define what environment variables can    #
# be reported via the env_report directive.  This was implemented to fix     #
# the problem reported at http://www.securityfocus.com/bid/1187              #

@valid_ENV = ('REMOTE_HOST','REMOTE_ADDR','REMOTE_USER','HTTP_USER_AGENT');

# Done                                                                       #
##############################################################################

# Check Referring URL
&check_url;

# Retrieve Date
&get_date;

# Parse Form Contents
&parse_form;

# Check Required Fields
&check_required;

# Send E-Mail
&send_mail;

# Return HTML Page or Redirect User
&return_html;

# NOTE rev1.91: This function is no longer intended to stop abuse, that      #
#    functionality is now embedded in the checks made on @recipients and the #
#    recipient form field.                                                   #

sub check_url {

    # Localize the check_referer flag which determines if user is valid.     #
    local($check_referer) = 0;

    # If a referring URL was specified, for each valid referer, make sure    #
    # that a valid referring URL was passed to FormMail.                     #

    if ($ENV{'HTTP_REFERER'}) {
        foreach $referer (@referers) {
            if ($ENV{'HTTP_REFERER'} =~ m|https?://([^/]*)$referer|i) {
                $check_referer = 1;
                last;
            }
        }
    }
    else {
        $check_referer = 1;
    }

    # If the HTTP_REFERER was invalid, send back an error.                   #
    if ($check_referer != 1) { &error('bad_referer') }
}

sub get_date {

    # Define arrays for the day of the week and month of the year.           #
    @days   = ('Sunday','Monday','Tuesday','Wednesday',
               'Thursday','Friday','Saturday');
    @months = ('January','February','March','April','May','June','July',
               'August','September','October','November','December');

    # Get the current time and format the hour, minutes and seconds.  Add    #
    # 1900 to the year to get the full 4 digit year.                         #
    ($sec,$min,$hour,$mday,$mon,$year,$wday) = (localtime(time))[0,1,2,3,4,5,6];
    $time = sprintf("%02d:%02d:%02d",$hour,$min,$sec);
    $year += 1900;

    # Format the date.                                                       #
    $date = "$days[$wday], $months[$mon] $mday, $year at $time";

}

sub parse_form {

    # Define the configuration associative array.                            #
    %Config = ('recipient','',          'subject','',
               'email','',              'name','',
               'redirect','',           'bgcolor','',
               'background','',         'link_color','',
               'vlink_color','',        'text_color','',
               'alink_color','',        'title','',
               'sort','',               'print_config','',
               'required','',           'env_report','',
               'return_link_title','',  'return_link_url','',
               'print_blank_fields','', 'missing_fields_redirect','');

    # Determine the form's REQUEST_METHOD (GET or POST) and split the form   #
    # fields up into their name-value pairs.  If the REQUEST_METHOD was      #
    # not GET or POST, send an error.                                        #
    if ($ENV{'REQUEST_METHOD'} eq 'GET') {
        # Split the name-value pairs
        @pairs = split(/&/, $ENV{'QUERY_STRING'});
    }
    elsif ($ENV{'REQUEST_METHOD'} eq 'POST') {
        # Get the input
        read(STDIN, $buffer, $ENV{'CONTENT_LENGTH'});

        # Split the name-value pairs
        @pairs = split(/&/, $buffer);
    }
    else {
        &error('request_method');
    }

    # For each name-value pair:                                              #
    foreach $pair (@pairs) {

        # Split the pair up into individual variables.                       #
        local($name, $value) = split(/=/, $pair);

        # Decode the form encoding on the name and value variables.          #
        # v1.92: remove null bytes                                           #
        $name =~ tr/+/ /;
        $name =~ s/%([a-fA-F0-9][a-fA-F0-9])/pack("C", hex($1))/eg;
        $name =~ tr/\0//d;

        $value =~ tr/+/ /;
        $value =~ s/%([a-fA-F0-9][a-fA-F0-9])/pack("C", hex($1))/eg;
        $value =~ tr/\0//d;

        # If the field name has been specified in the %Config array, it will #
        # return a 1 for defined($Config{$name}}) and we should associate    #
        # this value with the appropriate configuration variable.  If this   #
        # is not a configuration form field, put it into the associative     #
        # array %Form, appending the value with a ', ' if there is already a #
        # value present.  We also save the order of the form fields in the   #
        # @Field_Order array so we can use this order for the generic sort.  #
        if (defined($Config{$name})) {
            $Config{$name} = $value;
        }
        else {
            if ($Form{$name} ne '') {
                $Form{$name} = "$Form{$name}, $value";
            }
            else {
                push(@Field_Order,$name);
                $Form{$name} = $value;
            }
        }
    }

    # The next six lines remove any extra spaces or new lines from the       #
    # configuration variables, which may have been caused if your editor     #
    # wraps lines after a certain length or if you used spaces between field #
    # names or environment variables.                                        #
    $Config{'required'} =~ s/(\s+|\n)?,(\s+|\n)?/,/g;
    $Config{'required'} =~ s/(\s+)?\n+(\s+)?//g;
    $Config{'env_report'} =~ s/(\s+|\n)?,(\s+|\n)?/,/g;
    $Config{'env_report'} =~ s/(\s+)?\n+(\s+)?//g;
    $Config{'print_config'} =~ s/(\s+|\n)?,(\s+|\n)?/,/g;
    $Config{'print_config'} =~ s/(\s+)?\n+(\s+)?//g;

    # Split the configuration variables into individual field names.         #
    @Required = split(/,/,$Config{'required'});
    @Env_Report = split(/,/,$Config{'env_report'});
    @Print_Config = split(/,/,$Config{'print_config'});

    # ACCESS CONTROL FIX: Only allow ENV variables in @valid_ENV in          #
    # @Env_Report for security reasons.                                      #
    foreach $env_item (@Env_Report) {
        foreach $valid_item (@valid_ENV) {
            if ( $env_item eq $valid_item ) { push(@temp_array, $env_item) }
        }
    }
    @Env_Report = @temp_array;
}

sub check_required {

    # Localize the variables used in this subroutine.                        #
    local($require, @error);

    # The following insures that there were no newlines in any fields which  #
    # will be used in the header.                                            #
    if ($Config{'subject'} =~ /(\n|\r)/m || $Config{'recipient'} =~ /(\n|\r)/m) {
        &error('invalid_headers');
    }

### PJC 7/19/04
#    if (!$Config{'recipient'}) {
#        if (!defined(%Form)) { &error('bad_referer') }
#        else                 { &error('no_recipient') }
#    }
#    else {
#        # This block of code requires that the recipient address end with    #
#        # a valid domain or e-mail address as defined in @recipients.        #
#        $valid_recipient = 0;
#        foreach $send_to (split(/,/,$Config{'recipient'})) {
#            foreach $recipient (@recipients) {
#                if ($send_to =~ /$recipient$/i) {
#                    push(@send_to,$send_to); last;
#                }
#            }
#        }
#        if ($#send_to < 0) { &error('no_recipient') }
#        $Config{'recipient'} = join(',',@send_to);
#    }

    # For each require field defined in the form:                            #
    foreach $require (@Required) {

        # If the required field is the email field, the syntax of the email  #
        # address if checked to make sure it passes a valid syntax.          #
        if ($require eq 'email' && !&check_email($Config{$require})) {
            push(@error,$require);
        }

        # Otherwise, if the required field is a configuration field and it   #
        # has no value or has been filled in with a space, send an error.    #
        elsif (defined($Config{$require})) {
            if ($Config{$require} eq '') { push(@error,$require); }
        }

        # If it is a regular form field which has not been filled in or      #
        # filled in with a space, flag it as an error field.                 #
        elsif (!defined($Form{$require}) || $Form{$require} eq '') {
            push(@error,$require);
        }
    }

    # If any error fields have been found, send error message to the user.   #
    if (@error) { &error('missing_fields', @error) }
}

sub return_html {
    # Local variables used in this subroutine initialized.                   #
    local($key,$sort_order,$sorted_field);

    # Now that we have finished using form values for any e-mail related     #
    # reasons, we will convert all of the form fields and config values      #
    # to remove any cross-site scripting security holes.                     #
    local($field);
    foreach $field (keys %Config) {
        $safeConfig{$field} = &clean_html($Config{$field});
    }

    foreach $field (keys %Form) {
        $Form{$field} = &clean_html($Form{$field});
    }


    # If redirect option is used, print the redirectional location header.   #
    if ($Config{'redirect'}) {
        print "Location: $safeConfig{'redirect'}\n\n";
    }

    # Otherwise, begin printing the response page.                           #
    else {

        # Print HTTP header and opening HTML tags.                           #
        print "Content-type: text/html\n\n";
        print "<html>\n <head>\n";

        # Print out title of page                                            #
        if ($Config{'title'}) { print "<title>$safeConfig{'title'}</title>\n" }
        else                  { print "<title>Thank You</title>\n"        }

        print " </head>\n <body";

        # Get Body Tag Attributes                                            #
        &body_attributes;

        # Close Body Tag                                                     #
        print ">\n  <center>\n";

        # Print custom or generic title.                                     #
        if ($Config{'title'}) { print "<h1>$safeConfig{'title'}</h1>\n" }
        else { print "<h1>Thank You For Filling Out This Form</h1>\n" }

        print "</center>\n";

        print "Below is what you submitted to on ";
        print "$date<p><hr size=1 width=75\%><p>\n";

        # If a sort order is specified, sort the form fields based on that.  #
        if ($Config{'sort'} =~ /^order:.*,.*/) {

            # Set the temporary $sort_order variable to the sorting order,   #
            # remove extraneous line breaks and spaces, remove the order:    #
            # directive and split the sort fields into an array.             #
            $sort_order = $Config{'sort'};
            $sort_order =~ s/(\s+|\n)?,(\s+|\n)?/,/g;
            $sort_order =~ s/(\s+)?\n+(\s+)?//g;
            $sort_order =~ s/order://;
            @sorted_fields = split(/,/, $sort_order);

            # For each sorted field, if it has a value or the print blank    #
            # fields option is turned on print the form field and value.     #
            foreach $sorted_field (@sorted_fields) {
                local $sfname = &clean_html($sorted_field);

                if ($Config{'print_blank_fields'} || $Form{$sorted_field} ne '') {
                    print "<b>$sfname:</b> $Form{$sorted_field}<p>\n";
                }
            }
        }

        # Otherwise, use the order the fields were sent, or alphabetic.      #
        else {

            # Sort alphabetically if requested.
            if ($Config{'sort'} eq 'alphabetic') {
                @Field_Order = sort @Field_Order;
            }

            # For each form field, if it has a value or the print blank      #
            # fields option is turned on print the form field and value.     #
            foreach $field (@Field_Order) {
                local $fname = &clean_html($field);

                if ($Config{'print_blank_fields'} || $Form{$field} ne '') {
                    print "<b>$fname:</b> $Form{$field}<p>\n";
                }
            }
        }

        print "<p><hr size=1 width=75%><p>\n";

        # Check for a Return Link and print one if found.                    #
        if ($Config{'return_link_url'} && $Config{'return_link_title'}) {
            print "<ul>\n";
            print "<li><a href=\"$safeConfig{'return_link_url'}\">$safeConfig{'return_link_title'}</a>\n";
            print "</ul>\n";
        }

        # Print the page footer.                                             #
        print <<"(END HTML FOOTER)";
        <hr size=1 width=75%><p>
        <center><font size=-1><a href="http://www.scriptarchive.com/formmail.html">FormMail</a> V1.92 &copy; 1995 - 2002  Matt Wright<br>
A Free Product of <a href="http://www.scriptarchive.com/">Matt's Script Archive, Inc.</a></font></center>
        </body>
       </html>
(END HTML FOOTER)
    }
}

sub send_mail {
    # Localize variables used in this subroutine.                            #
    local($print_config,$key,$sort_order,$sorted_field,$env_report);

    # Open The Mail Program
    open(MAIL,"|$mailprog");

    print MAIL "To: $recipient\n";
    print MAIL "From: noreply@playberrybots.com\n";

    # Check for Message Subject
    if ($Config{'subject'}) { print MAIL "Subject: $Config{'subject'}\n\n" }
    else                    { print MAIL "Subject: BerryBots web UI feedback\n\n" }

    print MAIL "Date:  $date\n";
    print MAIL "Name:  $Config{'name'}\n";
    print MAIL "Email: $Config{'email'}\n";
    print MAIL "-" x 75 . "\n\n";

    if (@Print_Config) {
        foreach $print_config (@Print_Config) {
            if ($Config{$print_config}) {
                print MAIL "$print_config: $Config{$print_config}\n\n";
            }
        }
    }

    # If a sort order is specified, sort the form fields based on that.      #
    if ($Config{'sort'} =~ /^order:.*,.*/) {

        # Remove extraneous line breaks and spaces, remove the order:        #
        # directive and split the sort fields into an array.                 #
        local $sort_order = $Config{'sort'};
        $sort_order =~ s/(\s+|\n)?,(\s+|\n)?/,/g;
        $sort_order =~ s/(\s+)?\n+(\s+)?//g;
        $sort_order =~ s/order://;
        @sorted_fields = split(/,/, $sort_order);

        # For each sorted field, if it has a value or the print blank        #
        # fields option is turned on print the form field and value.         #
        foreach $sorted_field (@sorted_fields) {
            if ($Config{'print_blank_fields'} || $Form{$sorted_field} ne '') {
                print MAIL "$sorted_field: $Form{$sorted_field}\n\n";
            }
        }
    }

    # Otherwise, print fields in order they were sent or alphabetically.     #
    else {

        # Sort alphabetically if specified:                                  #
        if ($Config{'sort'} eq 'alphabetic') {
            @Field_Order = sort @Field_Order;
        }

        # For each form field, if it has a value or the print blank          #
        # fields option is turned on print the form field and value.         #
        foreach $field (@Field_Order) {
            if ($Config{'print_blank_fields'} || $Form{$field} ne '') {
                print MAIL "$field: $Form{$field}\n\n";
            }
        }
    }

    print MAIL "-" x 75 . "\n\n";

    # Send any specified Environment Variables to recipient.                 #
    foreach $env_report (@Env_Report) {
        if ($ENV{$env_report}) {
            print MAIL "$env_report: $ENV{$env_report}\n";
        }
    }

    close (MAIL);
}

sub check_email {
    # Initialize local email variable with input to subroutine.              #
    $email = $_[0];

    # If the e-mail address contains:                                        #
    if ($email =~ /(@.*@)|(\.\.)|(@\.)|(\.@)|(^\.)/ ||

        # the e-mail address contains an invalid syntax.  Or, if the         #
        # syntax does not match the following regular expression pattern     #
        # it fails basic syntax verification.                                #

        $email !~ /^.+\@(\[?)[a-zA-Z0-9\-\.]+\.([a-zA-Z0-9]+)(\]?)$/) {

        # Basic syntax requires:  one or more characters before the @ sign,  #
        # followed by an optional '[', then any number of letters, numbers,  #
        # dashes or periods (valid domain/IP characters) ending in a period  #
        # and then 2 or 3 letters (for domain suffixes) or 1 to 3 numbers    #
        # (for IP addresses).  An ending bracket is also allowed as it is    #
        # valid syntax to have an email address like: user@[255.255.255.0]   #

        # Return a false value, since the e-mail address did not pass valid  #
        # syntax.                                                            #
        return 0;
    }

    else {

        # Return a true value, e-mail verification passed.                   #
        return 1;
    }
}

# This was added into v1.91 to further secure the recipients array.  Now, by #
# default it will assume that valid recipients include only users with       #
# usernames A-Z, a-z, 0-9, _ and - that match your domain exactly.  If this  #
# is not what you want, you should read more detailed instructions regarding #
# the configuration of the @recipients variable in the documentation.        #
sub fill_recipients {
    local(@domains) = @_;
    local($domain,@return_recips);

    foreach $domain (@domains) {
        if ($domain =~ /^\d+\.\d+\.\d+\.\d+$/) {
            $domain =~ s/\./\\\./g;
            push(@return_recips,'^[\w\-\.]+\@\[' . $domain . '\]');
        }
        else {
            $domain =~ s/\./\\\./g;
            $domain =~ s/\-/\\\-/g;
            push(@return_recips,'^[\w\-\.]+\@' . $domain);
        }
    }

    return @return_recips;
}

# This function will convert <, >, & and " to their HTML equivalents.        #
sub clean_html {
    local $value = $_[0];
    $value =~ s/\&/\&amp;/g;
    $value =~ s/</\&lt;/g;
    $value =~ s/>/\&gt;/g;
    $value =~ s/"/\&quot;/g;
    return $value;
}

sub body_attributes {
    # Check for Background Color
    if ($Config{'bgcolor'}) { print " bgcolor=\"$safeConfig{'bgcolor'}\"" }

    # Check for Background Image
    if ($Config{'background'}) { print " background=\"$safeConfig{'background'}\"" }

    # Check for Link Color
    if ($Config{'link_color'}) { print " link=\"$safeConfig{'link_color'}\"" }

    # Check for Visited Link Color
    if ($Config{'vlink_color'}) { print " vlink=\"$safeConfig{'vlink_color'}\"" }

    # Check for Active Link Color
    if ($Config{'alink_color'}) { print " alink=\"$safeConfig{'alink_color'}\"" }

    # Check for Body Text Color
    if ($Config{'text_color'}) { print " text=\"$safeConfig{'text_color'}\"" }
}

sub error {
    # Localize variables and assign subroutine input.                        #
    local($error,@error_fields) = @_;
    local($host,$missing_field,$missing_field_list);

    if ($error eq 'bad_referer') {
        if ($ENV{'HTTP_REFERER'} =~ m|^https?://([\w\.]+)|i) {
            $host = $1;
            my $referer = &clean_html($ENV{'HTTP_REFERER'});
            print <<"(END ERROR HTML)";
Content-type: text/html

<html>
 <head>
  <title>Bad Referrer - Access Denied</title>
 </head>
 <body bgcolor=#FFFFFF text=#000000>
  <center>
   <table border=0 width=600 bgcolor=#9C9C9C>
    <tr><th><font size=+2>Bad Referrer - Access Denied</font></th></tr>
   </table>
   <table border=0 width=600 bgcolor=#CFCFCF>
    <tr><td>The form attempting to use
     <a href="http://www.scriptarchive.com/formmail.html">FormMail</a>
     resides at <tt>$referer</tt>, which is not allowed to access
     this cgi script.<p>

     If you are attempting to configure FormMail to run with this form, you need
     to add the following to \@referers, explained in detail in the
     <a href="http://www.scriptarchive.com/readme/formmail.html">README</a> file.<p>

     Add <tt>'$host'</tt> to your <tt><b>\@referers</b></tt> array.<hr size=1>
     <center><font size=-1>
      <a href="http://www.scriptarchive.com/formmail.html">FormMail</a> V1.92 &copy; 1995 - 2002  Matt Wright<br>
      A Free Product of <a href="http://www.scriptarchive.com/">Matt's Script Archive, Inc.</a>
     </font></center>
    </td></tr>
   </table>
  </center>
 </body>
</html>
(END ERROR HTML)
        }
        else {
            print <<"(END ERROR HTML)";
Content-type: text/html

<html>
 <head>
  <title>FormMail v1.92</title>
 </head>
 <body bgcolor=#FFFFFF text=#000000>
  <center>
   <table border=0 width=600 bgcolor=#9C9C9C>
    <tr><th><font size=+2>FormMail</font></th></tr>
   </table>
   <table border=0 width=600 bgcolor=#CFCFCF>
    <tr><th><tt><font size=+1>Copyright 1995 - 2002 Matt Wright<br>
        Version 1.92 - Released April 21, 2002<br>
        A Free Product of <a href="http://www.scriptarchive.com/">Matt's Script Archive,
        Inc.</a></font></tt></th></tr>
   </table>
  </center>
 </body>
</html>
(END ERROR HTML)
        }
    }

    elsif ($error eq 'request_method') {
            print <<"(END ERROR HTML)";
Content-type: text/html

<html>
 <head>
  <title>Error: Request Method</title>
 </head>
 <body bgcolor=#FFFFFF text=#000000>
  <center>
   <table border=0 width=600 bgcolor=#9C9C9C>
    <tr><th><font size=+2>Error: Request Method</font></th></tr>
   </table>
   <table border=0 width=600 bgcolor=#CFCFCF>
    <tr><td>The Request Method of the Form you submitted did not match
     either <tt>GET</tt> or <tt>POST</tt>.  Please check the form and make sure the
     <tt>method=</tt> statement is in upper case and matches <tt>GET</tt> or <tt>POST</tt>.<p>

     <center><font size=-1>
      <a href="http://www.scriptarchive.com/formmail.html">FormMail</a> V1.92 &copy; 1995 - 2002  Matt Wright<br>
      A Free Product of <a href="http://www.scriptarchive.com/">Matt's Script Archive, Inc.</a>
     </font></center>
    </td></tr>
   </table>
  </center>
 </body>
</html>
(END ERROR HTML)
    }

    elsif ($error eq 'no_recipient') {
            print <<"(END ERROR HTML)";
Content-type: text/html

<html>
 <head>
  <title>Error: Bad/No Recipient</title>
 </head>
 <body bgcolor=#FFFFFF text=#000000>
  <center>
   <table border=0 width=600 bgcolor=#9C9C9C>
    <tr><th><font size=+2>Error: Bad/No Recipient</font></th></tr>
   </table>
   <table border=0 width=600 bgcolor=#CFCFCF>
    <tr><td>There was no recipient or an invalid recipient specified in the data sent to FormMail.  Please
     make sure you have filled in the <tt>recipient</tt> form field with an e-mail
     address that has been configured in <tt>\@recipients</tt>.  More information on filling in <tt>recipient</tt> form fields and variables can be
     found in the <a href="http://www.scriptarchive.com/readme/formmail.html">README</a> file.<hr size=1>

     <center><font size=-1>
      <a href="http://www.scriptarchive.com/formmail.html">FormMail</a> V1.92 &copy; 1995 - 2002  Matt Wright<br>
      A Free Product of <a href="http://www.scriptarchive.com/">Matt's Script Archive, Inc.</a>
     </font></center>
    </td></tr>
   </table>
  </center>
 </body>
</html>
(END ERROR HTML)
    }

    elsif ($error eq 'invalid_headers') {
            print <<"(END ERROR HTML)";
Content-type: text/html

<html>
 <head>
  <title>Error: Bad Header Fields</title>
 </head>
 <body bgcolor=#FFFFFF text=#000000>
  <center>
   <table border=0 width=600 bgcolor=#9C9C9C>
    <tr><th><font size=+2>Error: Bad Header Fields</font></th></tr>
   </table>
   <table border=0 width=600 bgcolor=#CFCFCF>
    <tr><td>The header fields, which include <tt>recipient</tt>, <tt>email</tt>, <tt>name</tt> and <tt>subject</tt> were
     filled in with invalid values. You may not include any newline characters in these parameters.
     More information on filling in these form fields and variables can be
     found in the <a href="http://www.scriptarchive.com/readme/formmail.html">README</a> file.<hr size=1>

     <center><font size=-1>
      <a href="http://www.scriptarchive.com/formmail.html">FormMail</a> V1.92 &copy; 1995 - 2002  Matt Wright<br>
      A Free Product of <a href="http://www.scriptarchive.com/">Matt's Script Archive, Inc.</a>
     </font></center>
    </td></tr>
   </table>
  </center>
 </body>
</html>
(END ERROR HTML)
    }

    elsif ($error eq 'missing_fields') {
        if ($Config{'missing_fields_redirect'}) {
            print "Location: " . &clean_html($Config{'missing_fields_redirect'}) . "\n\n";
        }
        else {
            foreach $missing_field (@error_fields) {
                $missing_field_list .= "<li>" . &clean_html($missing_field) . "\n";
            }

            print <<"(END ERROR HTML)";
Content-type: text/html

<html>
 <head>
  <title>Error: Blank Fields</title>
 </head>
  <center>
   <table border=0 width=600 bgcolor=#9C9C9C>
    <tr><th><font size=+2>Error: Blank Fields</font></th></tr>
   </table>
   <table border=0 width=600 bgcolor=#CFCFCF>
    <tr><td>The following fields were left blank in your submission form:<p>
     <ul>
$missing_field_list
     </ul><br>

     These fields must be filled in before you can successfully submit the form.<p>
     Please use your browser's back button to return to the form and try again.<hr size=1>
     <center><font size=-1>
      <a href="http://www.scriptarchive.com/formmail.html">FormMail</a> V1.92 &copy; 1995 - 2002  Matt Wright<br>
      A Free Product of <a href="http://www.scriptarchive.com/">Matt's Script Archive, Inc.</a>
     </font></center>
    </td></tr>
   </table>
  </center>
 </body>
</html>
(END ERROR HTML)
        }
    }

    exit;
}



