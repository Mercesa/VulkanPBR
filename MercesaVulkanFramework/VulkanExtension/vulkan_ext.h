





<!DOCTYPE html>
<html lang="en">
  <head>
    <meta charset="utf-8">
  <link rel="dns-prefetch" href="https://assets-cdn.github.com">
  <link rel="dns-prefetch" href="https://avatars0.githubusercontent.com">
  <link rel="dns-prefetch" href="https://avatars1.githubusercontent.com">
  <link rel="dns-prefetch" href="https://avatars2.githubusercontent.com">
  <link rel="dns-prefetch" href="https://avatars3.githubusercontent.com">
  <link rel="dns-prefetch" href="https://github-cloud.s3.amazonaws.com">
  <link rel="dns-prefetch" href="https://user-images.githubusercontent.com/">



  <link crossorigin="anonymous" href="https://assets-cdn.github.com/assets/frameworks-bedfc518345498ab3204d330c1727cde7e733526a09cd7df6867f6a231565091.css" integrity="sha256-vt/FGDRUmKsyBNMwwXJ83n5zNSagnNffaGf2ojFWUJE=" media="all" rel="stylesheet" />
  <link crossorigin="anonymous" href="https://assets-cdn.github.com/assets/github-980cd404854e87ee88b12c7281a0875365a8f966c834dc30f3f656011f7f4df6.css" integrity="sha256-mAzUBIVOh+6IsSxygaCHU2Wo+WbINNww8/ZWAR9/TfY=" media="all" rel="stylesheet" />
  
  
  
  

  <meta name="viewport" content="width=device-width">
  
  <title>Vulkan-Docs/vulkan_ext.h at 1.0 · KhronosGroup/Vulkan-Docs</title>
  <link rel="search" type="application/opensearchdescription+xml" href="/opensearch.xml" title="GitHub">
  <link rel="fluid-icon" href="https://github.com/fluidicon.png" title="GitHub">
  <meta property="fb:app_id" content="1401488693436528">

    
    <meta content="https://avatars0.githubusercontent.com/u/1608701?v=4&amp;s=400" property="og:image" /><meta content="GitHub" property="og:site_name" /><meta content="object" property="og:type" /><meta content="KhronosGroup/Vulkan-Docs" property="og:title" /><meta content="https://github.com/KhronosGroup/Vulkan-Docs" property="og:url" /><meta content="Vulkan-Docs - The Vulkan API Specification and related tools" property="og:description" />

  <link rel="assets" href="https://assets-cdn.github.com/">
  <link rel="web-socket" href="wss://live.github.com/_sockets/VjI6MTkwMTk5Nzg1OjUwYjE5MzlmMjM4OTIzNWJlYjZlMmJiMThlNTQ5ZmJhZDI2MGQwYzRhZTZlMDU1M2I5MTJlZWE5ZmM5MzViNTA=--0acf208d161d1a5b742e87b9ec9f8eddd88e1928">
  <meta name="pjax-timeout" content="1000">
  <link rel="sudo-modal" href="/sessions/sudo_modal">
  <meta name="request-id" content="DC49:43C2:6C774A:C0EC19:59B64DAE" data-pjax-transient>
  

  <meta name="selected-link" value="repo_source" data-pjax-transient>

  <meta name="google-site-verification" content="KT5gs8h0wvaagLKAVWq8bbeNwnZZK1r1XQysX3xurLU">
<meta name="google-site-verification" content="ZzhVyEFwb7w3e0-uOTltm8Jsck2F5StVihD0exw2fsA">
    <meta name="google-analytics" content="UA-3769691-2">

<meta content="collector.githubapp.com" name="octolytics-host" /><meta content="github" name="octolytics-app-id" /><meta content="https://collector.githubapp.com/github-external/browser_event" name="octolytics-event-url" /><meta content="DC49:43C2:6C774A:C0EC19:59B64DAE" name="octolytics-dimension-request_id" /><meta content="iad" name="octolytics-dimension-region_edge" /><meta content="iad" name="octolytics-dimension-region_render" /><meta content="1593102" name="octolytics-actor-id" /><meta content="Mercesa" name="octolytics-actor-login" /><meta content="374911cf3db04b0015e8b97770158b7bd53b672531a4c101a206f45a0ee6c3c5" name="octolytics-actor-hash" />
<meta content="/&lt;user-name&gt;/&lt;repo-name&gt;/blob/show" data-pjax-transient="true" name="analytics-location" />




  <meta class="js-ga-set" name="dimension1" content="Logged In">


  

      <meta name="hostname" content="github.com">
  <meta name="user-login" content="Mercesa">

      <meta name="expected-hostname" content="github.com">
    <meta name="js-proxy-site-detection-payload" content="NjE3NTJjYjgyMDRkZmZjNzcyYzlhMmNlNTZhZDI2MTUxNmMzMjlhNjMwMzg3NDczNjQxYjE1MDkzZjZiYjcyMXx7InJlbW90ZV9hZGRyZXNzIjoiODQuMTA1LjIzMy4yMjkiLCJyZXF1ZXN0X2lkIjoiREM0OTo0M0MyOjZDNzc0QTpDMEVDMTk6NTlCNjREQUUiLCJ0aW1lc3RhbXAiOjE1MDUxMTk2NjMsImhvc3QiOiJnaXRodWIuY29tIn0=">

    <meta name="enabled-features" content="UNIVERSE_BANNER">

  <meta name="html-safe-nonce" content="7ce5a0460e65d270ff90ab22c0c1d8423fd3b1cd">

  <meta http-equiv="x-pjax-version" content="374d3000b7ab48f823a919af6164f269">
  

      <link href="https://github.com/KhronosGroup/Vulkan-Docs/commits/1.0.atom" rel="alternate" title="Recent Commits to Vulkan-Docs:1.0" type="application/atom+xml">

  <meta name="description" content="Vulkan-Docs - The Vulkan API Specification and related tools">
  <meta name="go-import" content="github.com/KhronosGroup/Vulkan-Docs git https://github.com/KhronosGroup/Vulkan-Docs.git">

  <meta content="1608701" name="octolytics-dimension-user_id" /><meta content="KhronosGroup" name="octolytics-dimension-user_login" /><meta content="51506618" name="octolytics-dimension-repository_id" /><meta content="KhronosGroup/Vulkan-Docs" name="octolytics-dimension-repository_nwo" /><meta content="true" name="octolytics-dimension-repository_public" /><meta content="false" name="octolytics-dimension-repository_is_fork" /><meta content="51506618" name="octolytics-dimension-repository_network_root_id" /><meta content="KhronosGroup/Vulkan-Docs" name="octolytics-dimension-repository_network_root_nwo" /><meta content="false" name="octolytics-dimension-repository_explore_github_marketplace_ci_cta_shown" />


    <link rel="canonical" href="https://github.com/KhronosGroup/Vulkan-Docs/blob/1.0/src/ext_loader/vulkan_ext.h" data-pjax-transient>


  <meta name="browser-stats-url" content="https://api.github.com/_private/browser/stats">

  <meta name="browser-errors-url" content="https://api.github.com/_private/browser/errors">

  <link rel="mask-icon" href="https://assets-cdn.github.com/pinned-octocat.svg" color="#000000">
  <link rel="icon" type="image/x-icon" href="https://assets-cdn.github.com/favicon.ico">

<meta name="theme-color" content="#1e2327">


  <meta name="u2f-support" content="true">

  </head>

  <body class="logged-in env-production page-blob">
    

  <div class="position-relative js-header-wrapper ">
    <a href="#start-of-content" tabindex="1" class="bg-black text-white p-3 show-on-focus js-skip-to-content">Skip to content</a>
    <div id="js-pjax-loader-bar" class="pjax-loader-bar"><div class="progress"></div></div>

    
    
    



        
<header class="Header  f5" role="banner">
  <div class="d-flex px-3 flex-justify-between container-lg">
    <div class="d-flex flex-justify-between">
      <a class="header-logo-invertocat" href="https://github.com/" data-hotkey="g d" aria-label="Homepage" data-ga-click="Header, go to dashboard, icon:logo">
  <svg aria-hidden="true" class="octicon octicon-mark-github" height="32" version="1.1" viewBox="0 0 16 16" width="32"><path fill-rule="evenodd" d="M8 0C3.58 0 0 3.58 0 8c0 3.54 2.29 6.53 5.47 7.59.4.07.55-.17.55-.38 0-.19-.01-.82-.01-1.49-2.01.37-2.53-.49-2.69-.94-.09-.23-.48-.94-.82-1.13-.28-.15-.68-.52-.01-.53.63-.01 1.08.58 1.23.82.72 1.21 1.87.87 2.33.66.07-.52.28-.87.51-1.07-1.78-.2-3.64-.89-3.64-3.95 0-.87.31-1.59.82-2.15-.08-.2-.36-1.02.08-2.12 0 0 .67-.21 2.2.82.64-.18 1.32-.27 2-.27.68 0 1.36.09 2 .27 1.53-1.04 2.2-.82 2.2-.82.44 1.1.16 1.92.08 2.12.51.56.82 1.27.82 2.15 0 3.07-1.87 3.75-3.65 3.95.29.25.54.73.54 1.48 0 1.07-.01 1.93-.01 2.2 0 .21.15.46.55.38A8.013 8.013 0 0 0 16 8c0-4.42-3.58-8-8-8z"/></svg>
</a>


    </div>

    <div class="HeaderMenu d-flex flex-justify-between flex-auto">
      <div class="d-flex">
            <div class="">
              <div class="header-search scoped-search site-scoped-search js-site-search" role="search">
  <!-- '"` --><!-- </textarea></xmp> --></option></form><form accept-charset="UTF-8" action="/KhronosGroup/Vulkan-Docs/search" class="js-site-search-form" data-scoped-search-url="/KhronosGroup/Vulkan-Docs/search" data-unscoped-search-url="/search" method="get"><div style="margin:0;padding:0;display:inline"><input name="utf8" type="hidden" value="&#x2713;" /></div>
    <label class="form-control header-search-wrapper js-chromeless-input-container">
        <a href="/KhronosGroup/Vulkan-Docs/blob/1.0/src/ext_loader/vulkan_ext.h" class="header-search-scope no-underline">This repository</a>
      <input type="text"
        class="form-control header-search-input js-site-search-focus js-site-search-field is-clearable"
        data-hotkey="s"
        name="q"
        value=""
        placeholder="Search"
        aria-label="Search this repository"
        data-unscoped-placeholder="Search GitHub"
        data-scoped-placeholder="Search"
        autocapitalize="off">
        <input type="hidden" class="js-site-search-type-field" name="type" >
    </label>
</form></div>

            </div>

          <ul class="d-flex pl-2 flex-items-center text-bold list-style-none" role="navigation">
            <li>
              <a href="/pulls" aria-label="Pull requests you created" class="js-selected-navigation-item HeaderNavlink px-2" data-ga-click="Header, click, Nav menu - item:pulls context:user" data-hotkey="g p" data-selected-links="/pulls /pulls/assigned /pulls/mentioned /pulls">
                Pull requests
</a>            </li>
            <li>
              <a href="/issues" aria-label="Issues you created" class="js-selected-navigation-item HeaderNavlink px-2" data-ga-click="Header, click, Nav menu - item:issues context:user" data-hotkey="g i" data-selected-links="/issues /issues/assigned /issues/mentioned /issues">
                Issues
</a>            </li>
                <li>
                  <a href="/marketplace" class="js-selected-navigation-item HeaderNavlink px-2" data-ga-click="Header, click, Nav menu - item:marketplace context:user" data-selected-links=" /marketplace">
                    Marketplace
</a>                </li>
            <li>
              <a href="/explore" class="js-selected-navigation-item HeaderNavlink px-2" data-ga-click="Header, click, Nav menu - item:explore" data-selected-links="/explore /trending /trending/developers /integrations /integrations/feature/code /integrations/feature/collaborate /integrations/feature/ship showcases showcases_search showcases_landing /explore">
                Explore
</a>            </li>
          </ul>
      </div>

      <div class="d-flex">
        
<ul class="user-nav d-flex flex-items-center list-style-none" id="user-links">
  <li class="dropdown js-menu-container js-header-notifications">
    <span class="d-inline-block  px-2">
      
      <a href="/notifications" aria-label="You have no unread notifications" class="notification-indicator tooltipped tooltipped-s  js-socket-channel js-notification-indicator" data-channel="notification-changed:1593102" data-ga-click="Header, go to notifications, icon:read" data-hotkey="g n">
          <span class="mail-status "></span>
          <svg aria-hidden="true" class="octicon octicon-bell" height="16" version="1.1" viewBox="0 0 14 16" width="14"><path fill-rule="evenodd" d="M14 12v1H0v-1l.73-.58c.77-.77.81-2.55 1.19-4.42C2.69 3.23 6 2 6 2c0-.55.45-1 1-1s1 .45 1 1c0 0 3.39 1.23 4.16 5 .38 1.88.42 3.66 1.19 4.42l.66.58H14zm-7 4c1.11 0 2-.89 2-2H5c0 1.11.89 2 2 2z"/></svg>
</a>
    </span>
  </li>

  <li class="dropdown js-menu-container">
    <details class="dropdown-details js-dropdown-details d-flex px-2 flex-items-center">
      <summary class="HeaderNavlink"
         aria-label="Create new…"
         data-ga-click="Header, create new, icon:add">
        <svg aria-hidden="true" class="octicon octicon-plus float-left mr-1 mt-1" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 9H7v5H5V9H0V7h5V2h2v5h5z"/></svg>
        <span class="dropdown-caret mt-1"></span>
      </summary>

      <ul class="dropdown-menu dropdown-menu-sw">
        
<a class="dropdown-item" href="/new" data-ga-click="Header, create new repository">
  New repository
</a>

  <a class="dropdown-item" href="/new/import" data-ga-click="Header, import a repository">
    Import repository
  </a>

<a class="dropdown-item" href="https://gist.github.com/" data-ga-click="Header, create new gist">
  New gist
</a>

  <a class="dropdown-item" href="/organizations/new" data-ga-click="Header, create new organization">
    New organization
  </a>



  <div class="dropdown-divider"></div>
  <div class="dropdown-header">
    <span title="KhronosGroup/Vulkan-Docs">This repository</span>
  </div>
    <a class="dropdown-item" href="/KhronosGroup/Vulkan-Docs/issues/new" data-ga-click="Header, create new issue">
      New issue
    </a>

      </ul>
    </details>
  </li>

  <li class="dropdown js-menu-container">

    <details class="dropdown-details js-dropdown-details d-flex pl-2 flex-items-center">
      <summary class="HeaderNavlink name mt-1"
        aria-label="View profile and more"
        data-ga-click="Header, show menu, icon:avatar">
        <img alt="@Mercesa" class="avatar float-left mr-1" src="https://avatars1.githubusercontent.com/u/1593102?v=4&amp;s=40" height="20" width="20">
        <span class="dropdown-caret"></span>
      </summary>

      <ul class="dropdown-menu dropdown-menu-sw">
        <li class="dropdown-header header-nav-current-user css-truncate">
          Signed in as <strong class="css-truncate-target">Mercesa</strong>
        </li>

        <li class="dropdown-divider"></li>

        <li><a class="dropdown-item" href="/Mercesa" data-ga-click="Header, go to profile, text:your profile">
          Your profile
        </a></li>
        <li><a class="dropdown-item" href="/Mercesa?tab=stars" data-ga-click="Header, go to starred repos, text:your stars">
          Your stars
        </a></li>
          <li><a class="dropdown-item" href="https://gist.github.com/" data-ga-click="Header, your gists, text:your gists">Your Gists</a></li>

        <li class="dropdown-divider"></li>

        <li><a class="dropdown-item" href="https://help.github.com" data-ga-click="Header, go to help, text:help">
          Help
        </a></li>

        <li><a class="dropdown-item" href="/settings/profile" data-ga-click="Header, go to settings, icon:settings">
          Settings
        </a></li>

        <li><!-- '"` --><!-- </textarea></xmp> --></option></form><form accept-charset="UTF-8" action="/logout" class="logout-form" method="post"><div style="margin:0;padding:0;display:inline"><input name="utf8" type="hidden" value="&#x2713;" /><input name="authenticity_token" type="hidden" value="iL6jmtR1K59KpnVfPqlv7wlEV8z792pcu+2O+Z043v1Q+S+NbIaS6dem44wwfrBn3JUOEqYAL4BS4M9PkMRiUQ==" /></div>
          <button type="submit" class="dropdown-item dropdown-signout" data-ga-click="Header, sign out, icon:logout">
            Sign out
          </button>
        </form></li>
      </ul>
    </details>
  </li>
</ul>


        <!-- '"` --><!-- </textarea></xmp> --></option></form><form accept-charset="UTF-8" action="/logout" class="sr-only right-0" method="post"><div style="margin:0;padding:0;display:inline"><input name="utf8" type="hidden" value="&#x2713;" /><input name="authenticity_token" type="hidden" value="9ZxROZsk0eg9RO3HoVyyOKjmAoG94yGUPKnJ2theME0t290uI9donqBEexSvi22wfTdbX+AUZEjVpIhs1aKM4Q==" /></div>
          <button type="submit" class="dropdown-item dropdown-signout" data-ga-click="Header, sign out, icon:logout">
            Sign out
          </button>
</form>      </div>
    </div>
  </div>
</header>


      

  </div>

  <div id="start-of-content" class="show-on-focus"></div>

    <div id="js-flash-container">
</div>



  <div role="main">
        <div itemscope itemtype="http://schema.org/SoftwareSourceCode">
    <div id="js-repo-pjax-container" data-pjax-container>
      



  



    <div class="pagehead repohead instapaper_ignore readability-menu experiment-repo-nav">
      <div class="container repohead-details-container">

        <ul class="pagehead-actions">
  <li>
        <!-- '"` --><!-- </textarea></xmp> --></option></form><form accept-charset="UTF-8" action="/notifications/subscribe" class="js-social-container" data-autosubmit="true" data-remote="true" method="post"><div style="margin:0;padding:0;display:inline"><input name="utf8" type="hidden" value="&#x2713;" /><input name="authenticity_token" type="hidden" value="Nww+BDW6OfnEVURKHLJafjnDL5PmTgQDqRiME/G2RcJtyhGAsThvNmhwxK8Hi1509jNE+/5htVNnOYqCusJtQw==" /></div>      <input class="form-control" id="repository_id" name="repository_id" type="hidden" value="51506618" />

        <div class="select-menu js-menu-container js-select-menu">
          <a href="/KhronosGroup/Vulkan-Docs/subscription"
            class="btn btn-sm btn-with-count select-menu-button js-menu-target"
            role="button"
            aria-haspopup="true"
            aria-expanded="false"
            aria-label="Toggle repository notifications menu"
            data-ga-click="Repository, click Watch settings, action:blob#show">
            <span class="js-select-button">
                <svg aria-hidden="true" class="octicon octicon-eye" height="16" version="1.1" viewBox="0 0 16 16" width="16"><path fill-rule="evenodd" d="M8.06 2C3 2 0 8 0 8s3 6 8.06 6C13 14 16 8 16 8s-3-6-7.94-6zM8 12c-2.2 0-4-1.78-4-4 0-2.2 1.8-4 4-4 2.22 0 4 1.8 4 4 0 2.22-1.78 4-4 4zm2-4c0 1.11-.89 2-2 2-1.11 0-2-.89-2-2 0-1.11.89-2 2-2 1.11 0 2 .89 2 2z"/></svg>
                Watch
            </span>
          </a>
            <a class="social-count js-social-count"
              href="/KhronosGroup/Vulkan-Docs/watchers"
              aria-label="181 users are watching this repository">
              181
            </a>

        <div class="select-menu-modal-holder">
          <div class="select-menu-modal subscription-menu-modal js-menu-content">
            <div class="select-menu-header js-navigation-enable" tabindex="-1">
              <svg aria-label="Close" class="octicon octicon-x js-menu-close" height="16" role="img" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M7.48 8l3.75 3.75-1.48 1.48L6 9.48l-3.75 3.75-1.48-1.48L4.52 8 .77 4.25l1.48-1.48L6 6.52l3.75-3.75 1.48 1.48z"/></svg>
              <span class="select-menu-title">Notifications</span>
            </div>

              <div class="select-menu-list js-navigation-container" role="menu">

                <div class="select-menu-item js-navigation-item selected" role="menuitem" tabindex="0">
                  <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
                  <div class="select-menu-item-text">
                    <input checked="checked" id="do_included" name="do" type="radio" value="included" />
                    <span class="select-menu-item-heading">Not watching</span>
                    <span class="description">Be notified when participating or @mentioned.</span>
                    <span class="js-select-button-text hidden-select-button-text">
                      <svg aria-hidden="true" class="octicon octicon-eye" height="16" version="1.1" viewBox="0 0 16 16" width="16"><path fill-rule="evenodd" d="M8.06 2C3 2 0 8 0 8s3 6 8.06 6C13 14 16 8 16 8s-3-6-7.94-6zM8 12c-2.2 0-4-1.78-4-4 0-2.2 1.8-4 4-4 2.22 0 4 1.8 4 4 0 2.22-1.78 4-4 4zm2-4c0 1.11-.89 2-2 2-1.11 0-2-.89-2-2 0-1.11.89-2 2-2 1.11 0 2 .89 2 2z"/></svg>
                      Watch
                    </span>
                  </div>
                </div>

                <div class="select-menu-item js-navigation-item " role="menuitem" tabindex="0">
                  <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
                  <div class="select-menu-item-text">
                    <input id="do_subscribed" name="do" type="radio" value="subscribed" />
                    <span class="select-menu-item-heading">Watching</span>
                    <span class="description">Be notified of all conversations.</span>
                    <span class="js-select-button-text hidden-select-button-text">
                      <svg aria-hidden="true" class="octicon octicon-eye" height="16" version="1.1" viewBox="0 0 16 16" width="16"><path fill-rule="evenodd" d="M8.06 2C3 2 0 8 0 8s3 6 8.06 6C13 14 16 8 16 8s-3-6-7.94-6zM8 12c-2.2 0-4-1.78-4-4 0-2.2 1.8-4 4-4 2.22 0 4 1.8 4 4 0 2.22-1.78 4-4 4zm2-4c0 1.11-.89 2-2 2-1.11 0-2-.89-2-2 0-1.11.89-2 2-2 1.11 0 2 .89 2 2z"/></svg>
                        Unwatch
                    </span>
                  </div>
                </div>

                <div class="select-menu-item js-navigation-item " role="menuitem" tabindex="0">
                  <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
                  <div class="select-menu-item-text">
                    <input id="do_ignore" name="do" type="radio" value="ignore" />
                    <span class="select-menu-item-heading">Ignoring</span>
                    <span class="description">Never be notified.</span>
                    <span class="js-select-button-text hidden-select-button-text">
                      <svg aria-hidden="true" class="octicon octicon-mute" height="16" version="1.1" viewBox="0 0 16 16" width="16"><path fill-rule="evenodd" d="M8 2.81v10.38c0 .67-.81 1-1.28.53L3 10H1c-.55 0-1-.45-1-1V7c0-.55.45-1 1-1h2l3.72-3.72C7.19 1.81 8 2.14 8 2.81zm7.53 3.22l-1.06-1.06-1.97 1.97-1.97-1.97-1.06 1.06L11.44 8 9.47 9.97l1.06 1.06 1.97-1.97 1.97 1.97 1.06-1.06L13.56 8l1.97-1.97z"/></svg>
                        Stop ignoring
                    </span>
                  </div>
                </div>

              </div>

            </div>
          </div>
        </div>
</form>
  </li>

  <li>
    
  <div class="js-toggler-container js-social-container starring-container ">
    <!-- '"` --><!-- </textarea></xmp> --></option></form><form accept-charset="UTF-8" action="/KhronosGroup/Vulkan-Docs/unstar" class="starred" data-remote="true" method="post"><div style="margin:0;padding:0;display:inline"><input name="utf8" type="hidden" value="&#x2713;" /><input name="authenticity_token" type="hidden" value="rMGDVfQBvfhVrS++XgNE9h7EoQrjd9DGAZM0DHTRTsLrwps1LL5xH48d0qqAPWMLTiphQTMvyvQo7wrr/nzPXQ==" /></div>
      <button
        type="submit"
        class="btn btn-sm btn-with-count js-toggler-target"
        aria-label="Unstar this repository" title="Unstar KhronosGroup/Vulkan-Docs"
        data-ga-click="Repository, click unstar button, action:blob#show; text:Unstar">
        <svg aria-hidden="true" class="octicon octicon-star" height="16" version="1.1" viewBox="0 0 14 16" width="14"><path fill-rule="evenodd" d="M14 6l-4.9-.64L7 1 4.9 5.36 0 6l3.6 3.26L2.67 14 7 11.67 11.33 14l-.93-4.74z"/></svg>
        Unstar
      </button>
        <a class="social-count js-social-count" href="/KhronosGroup/Vulkan-Docs/stargazers"
           aria-label="1021 users starred this repository">
          1,021
        </a>
</form>
    <!-- '"` --><!-- </textarea></xmp> --></option></form><form accept-charset="UTF-8" action="/KhronosGroup/Vulkan-Docs/star" class="unstarred" data-remote="true" method="post"><div style="margin:0;padding:0;display:inline"><input name="utf8" type="hidden" value="&#x2713;" /><input name="authenticity_token" type="hidden" value="AASDLB4efaycEUrdCjN3NgsIfz6TzTt2bwgAJ3dymFqgJyD1pVCW8WvJ7D8bNIEKhfKRfvwugws/jVmB70NTRA==" /></div>
      <button
        type="submit"
        class="btn btn-sm btn-with-count js-toggler-target"
        aria-label="Star this repository" title="Star KhronosGroup/Vulkan-Docs"
        data-ga-click="Repository, click star button, action:blob#show; text:Star">
        <svg aria-hidden="true" class="octicon octicon-star" height="16" version="1.1" viewBox="0 0 14 16" width="14"><path fill-rule="evenodd" d="M14 6l-4.9-.64L7 1 4.9 5.36 0 6l3.6 3.26L2.67 14 7 11.67 11.33 14l-.93-4.74z"/></svg>
        Star
      </button>
        <a class="social-count js-social-count" href="/KhronosGroup/Vulkan-Docs/stargazers"
           aria-label="1021 users starred this repository">
          1,021
        </a>
</form>  </div>

  </li>

  <li>
          <a href="#fork-destination-box" class="btn btn-sm btn-with-count"
              title="Fork your own copy of KhronosGroup/Vulkan-Docs to your account"
              aria-label="Fork your own copy of KhronosGroup/Vulkan-Docs to your account"
              rel="facebox"
              data-ga-click="Repository, show fork modal, action:blob#show; text:Fork">
              <svg aria-hidden="true" class="octicon octicon-repo-forked" height="16" version="1.1" viewBox="0 0 10 16" width="10"><path fill-rule="evenodd" d="M8 1a1.993 1.993 0 0 0-1 3.72V6L5 8 3 6V4.72A1.993 1.993 0 0 0 2 1a1.993 1.993 0 0 0-1 3.72V6.5l3 3v1.78A1.993 1.993 0 0 0 5 15a1.993 1.993 0 0 0 1-3.72V9.5l3-3V4.72A1.993 1.993 0 0 0 8 1zM2 4.2C1.34 4.2.8 3.65.8 3c0-.65.55-1.2 1.2-1.2.65 0 1.2.55 1.2 1.2 0 .65-.55 1.2-1.2 1.2zm3 10c-.66 0-1.2-.55-1.2-1.2 0-.65.55-1.2 1.2-1.2.65 0 1.2.55 1.2 1.2 0 .65-.55 1.2-1.2 1.2zm3-10c-.66 0-1.2-.55-1.2-1.2 0-.65.55-1.2 1.2-1.2.65 0 1.2.55 1.2 1.2 0 .65-.55 1.2-1.2 1.2z"/></svg>
            Fork
          </a>

          <div id="fork-destination-box" style="display: none;">
            <h2 class="facebox-header" data-facebox-id="facebox-header">Where should we fork this repository?</h2>
            <include-fragment src=""
                class="js-fork-select-fragment fork-select-fragment"
                data-url="/KhronosGroup/Vulkan-Docs/fork?fragment=1">
              <img alt="Loading" height="64" src="https://assets-cdn.github.com/images/spinners/octocat-spinner-128.gif" width="64" />
            </include-fragment>
          </div>

    <a href="/KhronosGroup/Vulkan-Docs/network" class="social-count"
       aria-label="135 users forked this repository">
      135
    </a>
  </li>
</ul>

        <h1 class="public ">
  <svg aria-hidden="true" class="octicon octicon-repo" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M4 9H3V8h1v1zm0-3H3v1h1V6zm0-2H3v1h1V4zm0-2H3v1h1V2zm8-1v12c0 .55-.45 1-1 1H6v2l-1.5-1.5L3 16v-2H1c-.55 0-1-.45-1-1V1c0-.55.45-1 1-1h10c.55 0 1 .45 1 1zm-1 10H1v2h2v-1h3v1h5v-2zm0-10H2v9h9V1z"/></svg>
  <span class="author" itemprop="author"><a href="/KhronosGroup" class="url fn" rel="author">KhronosGroup</a></span><!--
--><span class="path-divider">/</span><!--
--><strong itemprop="name"><a href="/KhronosGroup/Vulkan-Docs" data-pjax="#js-repo-pjax-container">Vulkan-Docs</a></strong>

</h1>

      </div>
      <div class="container">
        
<nav class="reponav js-repo-nav js-sidenav-container-pjax"
     itemscope
     itemtype="http://schema.org/BreadcrumbList"
     role="navigation"
     data-pjax="#js-repo-pjax-container">

  <span itemscope itemtype="http://schema.org/ListItem" itemprop="itemListElement">
    <a href="/KhronosGroup/Vulkan-Docs" class="js-selected-navigation-item selected reponav-item" data-hotkey="g c" data-selected-links="repo_source repo_downloads repo_commits repo_releases repo_tags repo_branches /KhronosGroup/Vulkan-Docs" itemprop="url">
      <svg aria-hidden="true" class="octicon octicon-code" height="16" version="1.1" viewBox="0 0 14 16" width="14"><path fill-rule="evenodd" d="M9.5 3L8 4.5 11.5 8 8 11.5 9.5 13 14 8 9.5 3zm-5 0L0 8l4.5 5L6 11.5 2.5 8 6 4.5 4.5 3z"/></svg>
      <span itemprop="name">Code</span>
      <meta itemprop="position" content="1">
</a>  </span>

    <span itemscope itemtype="http://schema.org/ListItem" itemprop="itemListElement">
      <a href="/KhronosGroup/Vulkan-Docs/issues" class="js-selected-navigation-item reponav-item" data-hotkey="g i" data-selected-links="repo_issues repo_labels repo_milestones /KhronosGroup/Vulkan-Docs/issues" itemprop="url">
        <svg aria-hidden="true" class="octicon octicon-issue-opened" height="16" version="1.1" viewBox="0 0 14 16" width="14"><path fill-rule="evenodd" d="M7 2.3c3.14 0 5.7 2.56 5.7 5.7s-2.56 5.7-5.7 5.7A5.71 5.71 0 0 1 1.3 8c0-3.14 2.56-5.7 5.7-5.7zM7 1C3.14 1 0 4.14 0 8s3.14 7 7 7 7-3.14 7-7-3.14-7-7-7zm1 3H6v5h2V4zm0 6H6v2h2v-2z"/></svg>
        <span itemprop="name">Issues</span>
        <span class="Counter">78</span>
        <meta itemprop="position" content="2">
</a>    </span>

  <span itemscope itemtype="http://schema.org/ListItem" itemprop="itemListElement">
    <a href="/KhronosGroup/Vulkan-Docs/pulls" class="js-selected-navigation-item reponav-item" data-hotkey="g p" data-selected-links="repo_pulls /KhronosGroup/Vulkan-Docs/pulls" itemprop="url">
      <svg aria-hidden="true" class="octicon octicon-git-pull-request" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M11 11.28V5c-.03-.78-.34-1.47-.94-2.06C9.46 2.35 8.78 2.03 8 2H7V0L4 3l3 3V4h1c.27.02.48.11.69.31.21.2.3.42.31.69v6.28A1.993 1.993 0 0 0 10 15a1.993 1.993 0 0 0 1-3.72zm-1 2.92c-.66 0-1.2-.55-1.2-1.2 0-.65.55-1.2 1.2-1.2.65 0 1.2.55 1.2 1.2 0 .65-.55 1.2-1.2 1.2zM4 3c0-1.11-.89-2-2-2a1.993 1.993 0 0 0-1 3.72v6.56A1.993 1.993 0 0 0 2 15a1.993 1.993 0 0 0 1-3.72V4.72c.59-.34 1-.98 1-1.72zm-.8 10c0 .66-.55 1.2-1.2 1.2-.65 0-1.2-.55-1.2-1.2 0-.65.55-1.2 1.2-1.2.65 0 1.2.55 1.2 1.2zM2 4.2C1.34 4.2.8 3.65.8 3c0-.65.55-1.2 1.2-1.2.65 0 1.2.55 1.2 1.2 0 .65-.55 1.2-1.2 1.2z"/></svg>
      <span itemprop="name">Pull requests</span>
      <span class="Counter">15</span>
      <meta itemprop="position" content="3">
</a>  </span>

    <a href="/KhronosGroup/Vulkan-Docs/projects" class="js-selected-navigation-item reponav-item" data-selected-links="repo_projects new_repo_project repo_project /KhronosGroup/Vulkan-Docs/projects">
      <svg aria-hidden="true" class="octicon octicon-project" height="16" version="1.1" viewBox="0 0 15 16" width="15"><path fill-rule="evenodd" d="M10 12h3V2h-3v10zm-4-2h3V2H6v8zm-4 4h3V2H2v12zm-1 1h13V1H1v14zM14 0H1a1 1 0 0 0-1 1v14a1 1 0 0 0 1 1h13a1 1 0 0 0 1-1V1a1 1 0 0 0-1-1z"/></svg>
      Projects
      <span class="Counter" >0</span>
</a>
    <a href="/KhronosGroup/Vulkan-Docs/wiki" class="js-selected-navigation-item reponav-item" data-hotkey="g w" data-selected-links="repo_wiki /KhronosGroup/Vulkan-Docs/wiki">
      <svg aria-hidden="true" class="octicon octicon-book" height="16" version="1.1" viewBox="0 0 16 16" width="16"><path fill-rule="evenodd" d="M3 5h4v1H3V5zm0 3h4V7H3v1zm0 2h4V9H3v1zm11-5h-4v1h4V5zm0 2h-4v1h4V7zm0 2h-4v1h4V9zm2-6v9c0 .55-.45 1-1 1H9.5l-1 1-1-1H2c-.55 0-1-.45-1-1V3c0-.55.45-1 1-1h5.5l1 1 1-1H15c.55 0 1 .45 1 1zm-8 .5L7.5 3H2v9h6V3.5zm7-.5H9.5l-.5.5V12h6V3z"/></svg>
      Wiki
</a>

    <div class="reponav-dropdown js-menu-container">
      <button type="button" class="btn-link reponav-item reponav-dropdown js-menu-target " data-no-toggle aria-expanded="false" aria-haspopup="true">
        Insights
        <svg aria-hidden="true" class="octicon octicon-triangle-down v-align-middle text-gray" height="11" version="1.1" viewBox="0 0 12 16" width="8"><path fill-rule="evenodd" d="M0 5l6 6 6-6z"/></svg>
      </button>
      <div class="dropdown-menu-content js-menu-content">
        <div class="dropdown-menu dropdown-menu-sw">
          <a class="dropdown-item" href="/KhronosGroup/Vulkan-Docs/pulse" data-skip-pjax>
            <svg aria-hidden="true" class="octicon octicon-pulse" height="16" version="1.1" viewBox="0 0 14 16" width="14"><path fill-rule="evenodd" d="M11.5 8L8.8 5.4 6.6 8.5 5.5 1.6 2.38 8H0v2h3.6l.9-1.8.9 5.4L9 8.5l1.6 1.5H14V8z"/></svg>
            Pulse
          </a>
          <a class="dropdown-item" href="/KhronosGroup/Vulkan-Docs/graphs" data-skip-pjax>
            <svg aria-hidden="true" class="octicon octicon-graph" height="16" version="1.1" viewBox="0 0 16 16" width="16"><path fill-rule="evenodd" d="M16 14v1H0V0h1v14h15zM5 13H3V8h2v5zm4 0H7V3h2v10zm4 0h-2V6h2v7z"/></svg>
            Graphs
          </a>
        </div>
      </div>
    </div>
</nav>

      </div>
    </div>

<div class="container new-discussion-timeline experiment-repo-nav">
  <div class="repository-content">

    
  <a href="/KhronosGroup/Vulkan-Docs/blob/6aa7d7bd2c447a3f10433ccd3b38a82e2a587eac/src/ext_loader/vulkan_ext.h" class="d-none js-permalink-shortcut" data-hotkey="y">Permalink</a>

  <!-- blob contrib key: blob_contributors:v21:597f5d7d684028fd7bccb8c2caacbeed -->

  <div class="file-navigation js-zeroclipboard-container">
    
<div class="select-menu branch-select-menu js-menu-container js-select-menu float-left">
  <button class=" btn btn-sm select-menu-button js-menu-target css-truncate" data-hotkey="w"
    
    type="button" aria-label="Switch branches or tags" aria-expanded="false" aria-haspopup="true">
      <i>Branch:</i>
      <span class="js-select-button css-truncate-target">1.0</span>
  </button>

  <div class="select-menu-modal-holder js-menu-content js-navigation-container" data-pjax>

    <div class="select-menu-modal">
      <div class="select-menu-header">
        <svg aria-label="Close" class="octicon octicon-x js-menu-close" height="16" role="img" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M7.48 8l3.75 3.75-1.48 1.48L6 9.48l-3.75 3.75-1.48-1.48L4.52 8 .77 4.25l1.48-1.48L6 6.52l3.75-3.75 1.48 1.48z"/></svg>
        <span class="select-menu-title">Switch branches/tags</span>
      </div>

      <div class="select-menu-filters">
        <div class="select-menu-text-filter">
          <input type="text" aria-label="Filter branches/tags" id="context-commitish-filter-field" class="form-control js-filterable-field js-navigation-enable" placeholder="Filter branches/tags">
        </div>
        <div class="select-menu-tabs">
          <ul>
            <li class="select-menu-tab">
              <a href="#" data-tab-filter="branches" data-filter-placeholder="Filter branches/tags" class="js-select-menu-tab" role="tab">Branches</a>
            </li>
            <li class="select-menu-tab">
              <a href="#" data-tab-filter="tags" data-filter-placeholder="Find a tag…" class="js-select-menu-tab" role="tab">Tags</a>
            </li>
          </ul>
        </div>
      </div>

      <div class="select-menu-list select-menu-tab-bucket js-select-menu-tab-bucket" data-tab-filter="branches" role="menu">

        <div data-filterable-for="context-commitish-filter-field" data-filterable-type="substring">


            <a class="select-menu-item js-navigation-item js-navigation-open "
               href="/KhronosGroup/Vulkan-Docs/blob/1.0-VK_AMD_draw_indirect_count/src/ext_loader/vulkan_ext.h"
               data-name="1.0-VK_AMD_draw_indirect_count"
               data-skip-pjax="true"
               rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target js-select-menu-filter-text">
                1.0-VK_AMD_draw_indirect_count
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
               href="/KhronosGroup/Vulkan-Docs/blob/1.0-VK_AMD_gcn_shader/src/ext_loader/vulkan_ext.h"
               data-name="1.0-VK_AMD_gcn_shader"
               data-skip-pjax="true"
               rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target js-select-menu-filter-text">
                1.0-VK_AMD_gcn_shader
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
               href="/KhronosGroup/Vulkan-Docs/blob/1.0-VK_AMD_rasterization_order/src/ext_loader/vulkan_ext.h"
               data-name="1.0-VK_AMD_rasterization_order"
               data-skip-pjax="true"
               rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target js-select-menu-filter-text">
                1.0-VK_AMD_rasterization_order
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
               href="/KhronosGroup/Vulkan-Docs/blob/1.0-VK_AMD_shader_explicit_vertex_parameter/src/ext_loader/vulkan_ext.h"
               data-name="1.0-VK_AMD_shader_explicit_vertex_parameter"
               data-skip-pjax="true"
               rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target js-select-menu-filter-text">
                1.0-VK_AMD_shader_explicit_vertex_parameter
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
               href="/KhronosGroup/Vulkan-Docs/blob/1.0-VK_AMD_shader_trinary_minmax/src/ext_loader/vulkan_ext.h"
               data-name="1.0-VK_AMD_shader_trinary_minmax"
               data-skip-pjax="true"
               rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target js-select-menu-filter-text">
                1.0-VK_AMD_shader_trinary_minmax
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
               href="/KhronosGroup/Vulkan-Docs/blob/1.0-VK_EXT_debug_marker/src/ext_loader/vulkan_ext.h"
               data-name="1.0-VK_EXT_debug_marker"
               data-skip-pjax="true"
               rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target js-select-menu-filter-text">
                1.0-VK_EXT_debug_marker
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
               href="/KhronosGroup/Vulkan-Docs/blob/1.0-VK_EXT_debug_report/src/ext_loader/vulkan_ext.h"
               data-name="1.0-VK_EXT_debug_report"
               data-skip-pjax="true"
               rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target js-select-menu-filter-text">
                1.0-VK_EXT_debug_report
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
               href="/KhronosGroup/Vulkan-Docs/blob/1.0-VK_IMG_filter_cubic/src/ext_loader/vulkan_ext.h"
               data-name="1.0-VK_IMG_filter_cubic"
               data-skip-pjax="true"
               rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target js-select-menu-filter-text">
                1.0-VK_IMG_filter_cubic
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
               href="/KhronosGroup/Vulkan-Docs/blob/1.0-VK_KHR_android_surface/src/ext_loader/vulkan_ext.h"
               data-name="1.0-VK_KHR_android_surface"
               data-skip-pjax="true"
               rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target js-select-menu-filter-text">
                1.0-VK_KHR_android_surface
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
               href="/KhronosGroup/Vulkan-Docs/blob/1.0-VK_KHR_display_swapchain/src/ext_loader/vulkan_ext.h"
               data-name="1.0-VK_KHR_display_swapchain"
               data-skip-pjax="true"
               rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target js-select-menu-filter-text">
                1.0-VK_KHR_display_swapchain
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
               href="/KhronosGroup/Vulkan-Docs/blob/1.0-VK_KHR_display/src/ext_loader/vulkan_ext.h"
               data-name="1.0-VK_KHR_display"
               data-skip-pjax="true"
               rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target js-select-menu-filter-text">
                1.0-VK_KHR_display
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
               href="/KhronosGroup/Vulkan-Docs/blob/1.0-VK_KHR_mir_surface/src/ext_loader/vulkan_ext.h"
               data-name="1.0-VK_KHR_mir_surface"
               data-skip-pjax="true"
               rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target js-select-menu-filter-text">
                1.0-VK_KHR_mir_surface
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
               href="/KhronosGroup/Vulkan-Docs/blob/1.0-VK_KHR_surface/src/ext_loader/vulkan_ext.h"
               data-name="1.0-VK_KHR_surface"
               data-skip-pjax="true"
               rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target js-select-menu-filter-text">
                1.0-VK_KHR_surface
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
               href="/KhronosGroup/Vulkan-Docs/blob/1.0-VK_KHR_swapchain/src/ext_loader/vulkan_ext.h"
               data-name="1.0-VK_KHR_swapchain"
               data-skip-pjax="true"
               rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target js-select-menu-filter-text">
                1.0-VK_KHR_swapchain
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
               href="/KhronosGroup/Vulkan-Docs/blob/1.0-VK_KHR_wayland_surface/src/ext_loader/vulkan_ext.h"
               data-name="1.0-VK_KHR_wayland_surface"
               data-skip-pjax="true"
               rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target js-select-menu-filter-text">
                1.0-VK_KHR_wayland_surface
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
               href="/KhronosGroup/Vulkan-Docs/blob/1.0-VK_KHR_win32_surface/src/ext_loader/vulkan_ext.h"
               data-name="1.0-VK_KHR_win32_surface"
               data-skip-pjax="true"
               rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target js-select-menu-filter-text">
                1.0-VK_KHR_win32_surface
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
               href="/KhronosGroup/Vulkan-Docs/blob/1.0-VK_KHR_xcb_surface/src/ext_loader/vulkan_ext.h"
               data-name="1.0-VK_KHR_xcb_surface"
               data-skip-pjax="true"
               rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target js-select-menu-filter-text">
                1.0-VK_KHR_xcb_surface
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
               href="/KhronosGroup/Vulkan-Docs/blob/1.0-VK_KHR_xlib_surface/src/ext_loader/vulkan_ext.h"
               data-name="1.0-VK_KHR_xlib_surface"
               data-skip-pjax="true"
               rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target js-select-menu-filter-text">
                1.0-VK_KHR_xlib_surface
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
               href="/KhronosGroup/Vulkan-Docs/blob/1.0-VK_NV_dedicated_allocation/src/ext_loader/vulkan_ext.h"
               data-name="1.0-VK_NV_dedicated_allocation"
               data-skip-pjax="true"
               rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target js-select-menu-filter-text">
                1.0-VK_NV_dedicated_allocation
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
               href="/KhronosGroup/Vulkan-Docs/blob/1.0-VK_NV_glsl_shader/src/ext_loader/vulkan_ext.h"
               data-name="1.0-VK_NV_glsl_shader"
               data-skip-pjax="true"
               rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target js-select-menu-filter-text">
                1.0-VK_NV_glsl_shader
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
               href="/KhronosGroup/Vulkan-Docs/blob/1.0-wsi_extensions/src/ext_loader/vulkan_ext.h"
               data-name="1.0-wsi_extensions"
               data-skip-pjax="true"
               rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target js-select-menu-filter-text">
                1.0-wsi_extensions
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open selected"
               href="/KhronosGroup/Vulkan-Docs/blob/1.0/src/ext_loader/vulkan_ext.h"
               data-name="1.0"
               data-skip-pjax="true"
               rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target js-select-menu-filter-text">
                1.0
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
               href="/KhronosGroup/Vulkan-Docs/blob/170410-debug_report_issues/src/ext_loader/vulkan_ext.h"
               data-name="170410-debug_report_issues"
               data-skip-pjax="true"
               rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target js-select-menu-filter-text">
                170410-debug_report_issues
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
               href="/KhronosGroup/Vulkan-Docs/blob/PR-issue-259/src/ext_loader/vulkan_ext.h"
               data-name="PR-issue-259"
               data-skip-pjax="true"
               rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target js-select-menu-filter-text">
                PR-issue-259
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
               href="/KhronosGroup/Vulkan-Docs/blob/bug-212-GetProcAddr/src/ext_loader/vulkan_ext.h"
               data-name="bug-212-GetProcAddr"
               data-skip-pjax="true"
               rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target js-select-menu-filter-text">
                bug-212-GetProcAddr
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
               href="/KhronosGroup/Vulkan-Docs/blob/github-PR225b-GetProcAddr/src/ext_loader/vulkan_ext.h"
               data-name="github-PR225b-GetProcAddr"
               data-skip-pjax="true"
               rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target js-select-menu-filter-text">
                github-PR225b-GetProcAddr
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
               href="/KhronosGroup/Vulkan-Docs/blob/mark-1.0-GH408-missing_debug_report_object_enums/src/ext_loader/vulkan_ext.h"
               data-name="mark-1.0-GH408-missing_debug_report_object_enums"
               data-skip-pjax="true"
               rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target js-select-menu-filter-text">
                mark-1.0-GH408-missing_debug_report_object_enums
              </span>
            </a>
        </div>

          <div class="select-menu-no-results">Nothing to show</div>
      </div>

      <div class="select-menu-list select-menu-tab-bucket js-select-menu-tab-bucket" data-tab-filter="tags">
        <div data-filterable-for="context-commitish-filter-field" data-filterable-type="substring">


            <a class="select-menu-item js-navigation-item js-navigation-open "
              href="/KhronosGroup/Vulkan-Docs/tree/v1.0.60-core/src/ext_loader/vulkan_ext.h"
              data-name="v1.0.60-core"
              data-skip-pjax="true"
              rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target" title="v1.0.60-core">
                v1.0.60-core
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
              href="/KhronosGroup/Vulkan-Docs/tree/v1.0.59-core/src/ext_loader/vulkan_ext.h"
              data-name="v1.0.59-core"
              data-skip-pjax="true"
              rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target" title="v1.0.59-core">
                v1.0.59-core
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
              href="/KhronosGroup/Vulkan-Docs/tree/v1.0.58-core/src/ext_loader/vulkan_ext.h"
              data-name="v1.0.58-core"
              data-skip-pjax="true"
              rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target" title="v1.0.58-core">
                v1.0.58-core
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
              href="/KhronosGroup/Vulkan-Docs/tree/v1.0.57-core/src/ext_loader/vulkan_ext.h"
              data-name="v1.0.57-core"
              data-skip-pjax="true"
              rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target" title="v1.0.57-core">
                v1.0.57-core
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
              href="/KhronosGroup/Vulkan-Docs/tree/v1.0.56-core/src/ext_loader/vulkan_ext.h"
              data-name="v1.0.56-core"
              data-skip-pjax="true"
              rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target" title="v1.0.56-core">
                v1.0.56-core
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
              href="/KhronosGroup/Vulkan-Docs/tree/v1.0.55-core/src/ext_loader/vulkan_ext.h"
              data-name="v1.0.55-core"
              data-skip-pjax="true"
              rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target" title="v1.0.55-core">
                v1.0.55-core
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
              href="/KhronosGroup/Vulkan-Docs/tree/v1.0.54-core/src/ext_loader/vulkan_ext.h"
              data-name="v1.0.54-core"
              data-skip-pjax="true"
              rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target" title="v1.0.54-core">
                v1.0.54-core
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
              href="/KhronosGroup/Vulkan-Docs/tree/v1.0.53-core/src/ext_loader/vulkan_ext.h"
              data-name="v1.0.53-core"
              data-skip-pjax="true"
              rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target" title="v1.0.53-core">
                v1.0.53-core
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
              href="/KhronosGroup/Vulkan-Docs/tree/v1.0.51-core/src/ext_loader/vulkan_ext.h"
              data-name="v1.0.51-core"
              data-skip-pjax="true"
              rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target" title="v1.0.51-core">
                v1.0.51-core
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
              href="/KhronosGroup/Vulkan-Docs/tree/v1.0.50-core/src/ext_loader/vulkan_ext.h"
              data-name="v1.0.50-core"
              data-skip-pjax="true"
              rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target" title="v1.0.50-core">
                v1.0.50-core
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
              href="/KhronosGroup/Vulkan-Docs/tree/v1.0.49-core/src/ext_loader/vulkan_ext.h"
              data-name="v1.0.49-core"
              data-skip-pjax="true"
              rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target" title="v1.0.49-core">
                v1.0.49-core
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
              href="/KhronosGroup/Vulkan-Docs/tree/v1.0.48-core/src/ext_loader/vulkan_ext.h"
              data-name="v1.0.48-core"
              data-skip-pjax="true"
              rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target" title="v1.0.48-core">
                v1.0.48-core
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
              href="/KhronosGroup/Vulkan-Docs/tree/v1.0.47-core/src/ext_loader/vulkan_ext.h"
              data-name="v1.0.47-core"
              data-skip-pjax="true"
              rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target" title="v1.0.47-core">
                v1.0.47-core
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
              href="/KhronosGroup/Vulkan-Docs/tree/v1.0.46-core/src/ext_loader/vulkan_ext.h"
              data-name="v1.0.46-core"
              data-skip-pjax="true"
              rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target" title="v1.0.46-core">
                v1.0.46-core
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
              href="/KhronosGroup/Vulkan-Docs/tree/v1.0.45-core/src/ext_loader/vulkan_ext.h"
              data-name="v1.0.45-core"
              data-skip-pjax="true"
              rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target" title="v1.0.45-core">
                v1.0.45-core
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
              href="/KhronosGroup/Vulkan-Docs/tree/v1.0.44-core/src/ext_loader/vulkan_ext.h"
              data-name="v1.0.44-core"
              data-skip-pjax="true"
              rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target" title="v1.0.44-core">
                v1.0.44-core
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
              href="/KhronosGroup/Vulkan-Docs/tree/v1.0.43-core/src/ext_loader/vulkan_ext.h"
              data-name="v1.0.43-core"
              data-skip-pjax="true"
              rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target" title="v1.0.43-core">
                v1.0.43-core
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
              href="/KhronosGroup/Vulkan-Docs/tree/v1.0.42-core/src/ext_loader/vulkan_ext.h"
              data-name="v1.0.42-core"
              data-skip-pjax="true"
              rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target" title="v1.0.42-core">
                v1.0.42-core
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
              href="/KhronosGroup/Vulkan-Docs/tree/v1.0.41-core/src/ext_loader/vulkan_ext.h"
              data-name="v1.0.41-core"
              data-skip-pjax="true"
              rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target" title="v1.0.41-core">
                v1.0.41-core
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
              href="/KhronosGroup/Vulkan-Docs/tree/v1.0.40-core/src/ext_loader/vulkan_ext.h"
              data-name="v1.0.40-core"
              data-skip-pjax="true"
              rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target" title="v1.0.40-core">
                v1.0.40-core
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
              href="/KhronosGroup/Vulkan-Docs/tree/v1.0.39-core/src/ext_loader/vulkan_ext.h"
              data-name="v1.0.39-core"
              data-skip-pjax="true"
              rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target" title="v1.0.39-core">
                v1.0.39-core
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
              href="/KhronosGroup/Vulkan-Docs/tree/v1.0.38-core/src/ext_loader/vulkan_ext.h"
              data-name="v1.0.38-core"
              data-skip-pjax="true"
              rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target" title="v1.0.38-core">
                v1.0.38-core
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
              href="/KhronosGroup/Vulkan-Docs/tree/v1.0.36-core/src/ext_loader/vulkan_ext.h"
              data-name="v1.0.36-core"
              data-skip-pjax="true"
              rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target" title="v1.0.36-core">
                v1.0.36-core
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
              href="/KhronosGroup/Vulkan-Docs/tree/v1.0.35-core/src/ext_loader/vulkan_ext.h"
              data-name="v1.0.35-core"
              data-skip-pjax="true"
              rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target" title="v1.0.35-core">
                v1.0.35-core
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
              href="/KhronosGroup/Vulkan-Docs/tree/v1.0.34-core/src/ext_loader/vulkan_ext.h"
              data-name="v1.0.34-core"
              data-skip-pjax="true"
              rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target" title="v1.0.34-core">
                v1.0.34-core
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
              href="/KhronosGroup/Vulkan-Docs/tree/v1.0.33-core/src/ext_loader/vulkan_ext.h"
              data-name="v1.0.33-core"
              data-skip-pjax="true"
              rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target" title="v1.0.33-core">
                v1.0.33-core
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
              href="/KhronosGroup/Vulkan-Docs/tree/v1.0-core+wsi-20160812/src/ext_loader/vulkan_ext.h"
              data-name="v1.0-core+wsi-20160812"
              data-skip-pjax="true"
              rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target" title="v1.0-core+wsi-20160812">
                v1.0-core+wsi-20160812
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
              href="/KhronosGroup/Vulkan-Docs/tree/v1.0-core+wsi-20160805/src/ext_loader/vulkan_ext.h"
              data-name="v1.0-core+wsi-20160805"
              data-skip-pjax="true"
              rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target" title="v1.0-core+wsi-20160805">
                v1.0-core+wsi-20160805
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
              href="/KhronosGroup/Vulkan-Docs/tree/v1.0-core+wsi-20160722/src/ext_loader/vulkan_ext.h"
              data-name="v1.0-core+wsi-20160722"
              data-skip-pjax="true"
              rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target" title="v1.0-core+wsi-20160722">
                v1.0-core+wsi-20160722
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
              href="/KhronosGroup/Vulkan-Docs/tree/v1.0-core+wsi-20160715/src/ext_loader/vulkan_ext.h"
              data-name="v1.0-core+wsi-20160715"
              data-skip-pjax="true"
              rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target" title="v1.0-core+wsi-20160715">
                v1.0-core+wsi-20160715
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
              href="/KhronosGroup/Vulkan-Docs/tree/v1.0-core+wsi-20160710/src/ext_loader/vulkan_ext.h"
              data-name="v1.0-core+wsi-20160710"
              data-skip-pjax="true"
              rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target" title="v1.0-core+wsi-20160710">
                v1.0-core+wsi-20160710
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
              href="/KhronosGroup/Vulkan-Docs/tree/v1.0-core+wsi-20160701/src/ext_loader/vulkan_ext.h"
              data-name="v1.0-core+wsi-20160701"
              data-skip-pjax="true"
              rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target" title="v1.0-core+wsi-20160701">
                v1.0-core+wsi-20160701
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
              href="/KhronosGroup/Vulkan-Docs/tree/v1.0-core+wsi-20160624/src/ext_loader/vulkan_ext.h"
              data-name="v1.0-core+wsi-20160624"
              data-skip-pjax="true"
              rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target" title="v1.0-core+wsi-20160624">
                v1.0-core+wsi-20160624
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
              href="/KhronosGroup/Vulkan-Docs/tree/v1.0-core+wsi-20160617/src/ext_loader/vulkan_ext.h"
              data-name="v1.0-core+wsi-20160617"
              data-skip-pjax="true"
              rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target" title="v1.0-core+wsi-20160617">
                v1.0-core+wsi-20160617
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
              href="/KhronosGroup/Vulkan-Docs/tree/v1.0-core+wsi-20160610/src/ext_loader/vulkan_ext.h"
              data-name="v1.0-core+wsi-20160610"
              data-skip-pjax="true"
              rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target" title="v1.0-core+wsi-20160610">
                v1.0-core+wsi-20160610
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
              href="/KhronosGroup/Vulkan-Docs/tree/v1.0-core+wsi-20160527/src/ext_loader/vulkan_ext.h"
              data-name="v1.0-core+wsi-20160527"
              data-skip-pjax="true"
              rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target" title="v1.0-core+wsi-20160527">
                v1.0-core+wsi-20160527
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
              href="/KhronosGroup/Vulkan-Docs/tree/v1.0-core+wsi-20160520/src/ext_loader/vulkan_ext.h"
              data-name="v1.0-core+wsi-20160520"
              data-skip-pjax="true"
              rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target" title="v1.0-core+wsi-20160520">
                v1.0-core+wsi-20160520
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
              href="/KhronosGroup/Vulkan-Docs/tree/v1.0-core+wsi-20160513/src/ext_loader/vulkan_ext.h"
              data-name="v1.0-core+wsi-20160513"
              data-skip-pjax="true"
              rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target" title="v1.0-core+wsi-20160513">
                v1.0-core+wsi-20160513
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
              href="/KhronosGroup/Vulkan-Docs/tree/v1.0-core+wsi-20160429/src/ext_loader/vulkan_ext.h"
              data-name="v1.0-core+wsi-20160429"
              data-skip-pjax="true"
              rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target" title="v1.0-core+wsi-20160429">
                v1.0-core+wsi-20160429
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
              href="/KhronosGroup/Vulkan-Docs/tree/v1.0-core+wsi-20160422/src/ext_loader/vulkan_ext.h"
              data-name="v1.0-core+wsi-20160422"
              data-skip-pjax="true"
              rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target" title="v1.0-core+wsi-20160422">
                v1.0-core+wsi-20160422
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
              href="/KhronosGroup/Vulkan-Docs/tree/v1.0-core+wsi-20160415/src/ext_loader/vulkan_ext.h"
              data-name="v1.0-core+wsi-20160415"
              data-skip-pjax="true"
              rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target" title="v1.0-core+wsi-20160415">
                v1.0-core+wsi-20160415
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
              href="/KhronosGroup/Vulkan-Docs/tree/v1.0-core+wsi-20160408/src/ext_loader/vulkan_ext.h"
              data-name="v1.0-core+wsi-20160408"
              data-skip-pjax="true"
              rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target" title="v1.0-core+wsi-20160408">
                v1.0-core+wsi-20160408
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
              href="/KhronosGroup/Vulkan-Docs/tree/v1.0-core+wsi-20160401/src/ext_loader/vulkan_ext.h"
              data-name="v1.0-core+wsi-20160401"
              data-skip-pjax="true"
              rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target" title="v1.0-core+wsi-20160401">
                v1.0-core+wsi-20160401
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
              href="/KhronosGroup/Vulkan-Docs/tree/v1.0-core+wsi-20160325/src/ext_loader/vulkan_ext.h"
              data-name="v1.0-core+wsi-20160325"
              data-skip-pjax="true"
              rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target" title="v1.0-core+wsi-20160325">
                v1.0-core+wsi-20160325
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
              href="/KhronosGroup/Vulkan-Docs/tree/v1.0-core+wsi-20160311/src/ext_loader/vulkan_ext.h"
              data-name="v1.0-core+wsi-20160311"
              data-skip-pjax="true"
              rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target" title="v1.0-core+wsi-20160311">
                v1.0-core+wsi-20160311
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
              href="/KhronosGroup/Vulkan-Docs/tree/v1.0-core+wsi-20160310/src/ext_loader/vulkan_ext.h"
              data-name="v1.0-core+wsi-20160310"
              data-skip-pjax="true"
              rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target" title="v1.0-core+wsi-20160310">
                v1.0-core+wsi-20160310
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
              href="/KhronosGroup/Vulkan-Docs/tree/v1.0-core+wsi-20160304/src/ext_loader/vulkan_ext.h"
              data-name="v1.0-core+wsi-20160304"
              data-skip-pjax="true"
              rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target" title="v1.0-core+wsi-20160304">
                v1.0-core+wsi-20160304
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
              href="/KhronosGroup/Vulkan-Docs/tree/v1.0-core+wsi-20160226/src/ext_loader/vulkan_ext.h"
              data-name="v1.0-core+wsi-20160226"
              data-skip-pjax="true"
              rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target" title="v1.0-core+wsi-20160226">
                v1.0-core+wsi-20160226
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
              href="/KhronosGroup/Vulkan-Docs/tree/v1.0-core+wsi-20160216/src/ext_loader/vulkan_ext.h"
              data-name="v1.0-core+wsi-20160216"
              data-skip-pjax="true"
              rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target" title="v1.0-core+wsi-20160216">
                v1.0-core+wsi-20160216
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
              href="/KhronosGroup/Vulkan-Docs/tree/v1.0-core-20161025/src/ext_loader/vulkan_ext.h"
              data-name="v1.0-core-20161025"
              data-skip-pjax="true"
              rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target" title="v1.0-core-20161025">
                v1.0-core-20161025
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
              href="/KhronosGroup/Vulkan-Docs/tree/v1.0-core-20161014/src/ext_loader/vulkan_ext.h"
              data-name="v1.0-core-20161014"
              data-skip-pjax="true"
              rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target" title="v1.0-core-20161014">
                v1.0-core-20161014
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
              href="/KhronosGroup/Vulkan-Docs/tree/v1.0-core-20161007/src/ext_loader/vulkan_ext.h"
              data-name="v1.0-core-20161007"
              data-skip-pjax="true"
              rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target" title="v1.0-core-20161007">
                v1.0-core-20161007
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
              href="/KhronosGroup/Vulkan-Docs/tree/v1.0-core-20160930/src/ext_loader/vulkan_ext.h"
              data-name="v1.0-core-20160930"
              data-skip-pjax="true"
              rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target" title="v1.0-core-20160930">
                v1.0-core-20160930
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
              href="/KhronosGroup/Vulkan-Docs/tree/v1.0-core-20160923/src/ext_loader/vulkan_ext.h"
              data-name="v1.0-core-20160923"
              data-skip-pjax="true"
              rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target" title="v1.0-core-20160923">
                v1.0-core-20160923
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
              href="/KhronosGroup/Vulkan-Docs/tree/v1.0-core-20160916/src/ext_loader/vulkan_ext.h"
              data-name="v1.0-core-20160916"
              data-skip-pjax="true"
              rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target" title="v1.0-core-20160916">
                v1.0-core-20160916
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
              href="/KhronosGroup/Vulkan-Docs/tree/v1.0-core-20160906/src/ext_loader/vulkan_ext.h"
              data-name="v1.0-core-20160906"
              data-skip-pjax="true"
              rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target" title="v1.0-core-20160906">
                v1.0-core-20160906
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
              href="/KhronosGroup/Vulkan-Docs/tree/v1.0-core-20160826/src/ext_loader/vulkan_ext.h"
              data-name="v1.0-core-20160826"
              data-skip-pjax="true"
              rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target" title="v1.0-core-20160826">
                v1.0-core-20160826
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
              href="/KhronosGroup/Vulkan-Docs/tree/v1.0-core-20160812/src/ext_loader/vulkan_ext.h"
              data-name="v1.0-core-20160812"
              data-skip-pjax="true"
              rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target" title="v1.0-core-20160812">
                v1.0-core-20160812
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
              href="/KhronosGroup/Vulkan-Docs/tree/v1.0-core-20160805/src/ext_loader/vulkan_ext.h"
              data-name="v1.0-core-20160805"
              data-skip-pjax="true"
              rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target" title="v1.0-core-20160805">
                v1.0-core-20160805
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
              href="/KhronosGroup/Vulkan-Docs/tree/v1.0-core-20160722/src/ext_loader/vulkan_ext.h"
              data-name="v1.0-core-20160722"
              data-skip-pjax="true"
              rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target" title="v1.0-core-20160722">
                v1.0-core-20160722
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
              href="/KhronosGroup/Vulkan-Docs/tree/v1.0-core-20160715/src/ext_loader/vulkan_ext.h"
              data-name="v1.0-core-20160715"
              data-skip-pjax="true"
              rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target" title="v1.0-core-20160715">
                v1.0-core-20160715
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
              href="/KhronosGroup/Vulkan-Docs/tree/v1.0-core-20160710/src/ext_loader/vulkan_ext.h"
              data-name="v1.0-core-20160710"
              data-skip-pjax="true"
              rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target" title="v1.0-core-20160710">
                v1.0-core-20160710
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
              href="/KhronosGroup/Vulkan-Docs/tree/v1.0-core-20160701/src/ext_loader/vulkan_ext.h"
              data-name="v1.0-core-20160701"
              data-skip-pjax="true"
              rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target" title="v1.0-core-20160701">
                v1.0-core-20160701
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
              href="/KhronosGroup/Vulkan-Docs/tree/v1.0-core-20160624/src/ext_loader/vulkan_ext.h"
              data-name="v1.0-core-20160624"
              data-skip-pjax="true"
              rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target" title="v1.0-core-20160624">
                v1.0-core-20160624
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
              href="/KhronosGroup/Vulkan-Docs/tree/v1.0-core-20160617/src/ext_loader/vulkan_ext.h"
              data-name="v1.0-core-20160617"
              data-skip-pjax="true"
              rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target" title="v1.0-core-20160617">
                v1.0-core-20160617
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
              href="/KhronosGroup/Vulkan-Docs/tree/v1.0-core-20160610/src/ext_loader/vulkan_ext.h"
              data-name="v1.0-core-20160610"
              data-skip-pjax="true"
              rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target" title="v1.0-core-20160610">
                v1.0-core-20160610
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
              href="/KhronosGroup/Vulkan-Docs/tree/v1.0-core-20160527/src/ext_loader/vulkan_ext.h"
              data-name="v1.0-core-20160527"
              data-skip-pjax="true"
              rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target" title="v1.0-core-20160527">
                v1.0-core-20160527
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
              href="/KhronosGroup/Vulkan-Docs/tree/v1.0-core-20160520/src/ext_loader/vulkan_ext.h"
              data-name="v1.0-core-20160520"
              data-skip-pjax="true"
              rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target" title="v1.0-core-20160520">
                v1.0-core-20160520
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
              href="/KhronosGroup/Vulkan-Docs/tree/v1.0-core-20160513/src/ext_loader/vulkan_ext.h"
              data-name="v1.0-core-20160513"
              data-skip-pjax="true"
              rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target" title="v1.0-core-20160513">
                v1.0-core-20160513
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
              href="/KhronosGroup/Vulkan-Docs/tree/v1.0-core-20160429/src/ext_loader/vulkan_ext.h"
              data-name="v1.0-core-20160429"
              data-skip-pjax="true"
              rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target" title="v1.0-core-20160429">
                v1.0-core-20160429
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
              href="/KhronosGroup/Vulkan-Docs/tree/v1.0-core-20160422/src/ext_loader/vulkan_ext.h"
              data-name="v1.0-core-20160422"
              data-skip-pjax="true"
              rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target" title="v1.0-core-20160422">
                v1.0-core-20160422
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
              href="/KhronosGroup/Vulkan-Docs/tree/v1.0-core-20160415/src/ext_loader/vulkan_ext.h"
              data-name="v1.0-core-20160415"
              data-skip-pjax="true"
              rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target" title="v1.0-core-20160415">
                v1.0-core-20160415
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
              href="/KhronosGroup/Vulkan-Docs/tree/v1.0-core-20160408/src/ext_loader/vulkan_ext.h"
              data-name="v1.0-core-20160408"
              data-skip-pjax="true"
              rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target" title="v1.0-core-20160408">
                v1.0-core-20160408
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
              href="/KhronosGroup/Vulkan-Docs/tree/v1.0-core-20160401/src/ext_loader/vulkan_ext.h"
              data-name="v1.0-core-20160401"
              data-skip-pjax="true"
              rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target" title="v1.0-core-20160401">
                v1.0-core-20160401
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
              href="/KhronosGroup/Vulkan-Docs/tree/v1.0-core-20160325/src/ext_loader/vulkan_ext.h"
              data-name="v1.0-core-20160325"
              data-skip-pjax="true"
              rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target" title="v1.0-core-20160325">
                v1.0-core-20160325
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
              href="/KhronosGroup/Vulkan-Docs/tree/v1.0-core-20160311/src/ext_loader/vulkan_ext.h"
              data-name="v1.0-core-20160311"
              data-skip-pjax="true"
              rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target" title="v1.0-core-20160311">
                v1.0-core-20160311
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
              href="/KhronosGroup/Vulkan-Docs/tree/v1.0-core-20160310/src/ext_loader/vulkan_ext.h"
              data-name="v1.0-core-20160310"
              data-skip-pjax="true"
              rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target" title="v1.0-core-20160310">
                v1.0-core-20160310
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
              href="/KhronosGroup/Vulkan-Docs/tree/v1.0-core-20160304/src/ext_loader/vulkan_ext.h"
              data-name="v1.0-core-20160304"
              data-skip-pjax="true"
              rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target" title="v1.0-core-20160304">
                v1.0-core-20160304
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
              href="/KhronosGroup/Vulkan-Docs/tree/v1.0-core-20160226/src/ext_loader/vulkan_ext.h"
              data-name="v1.0-core-20160226"
              data-skip-pjax="true"
              rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target" title="v1.0-core-20160226">
                v1.0-core-20160226
              </span>
            </a>
            <a class="select-menu-item js-navigation-item js-navigation-open "
              href="/KhronosGroup/Vulkan-Docs/tree/v1.0-core-20160216/src/ext_loader/vulkan_ext.h"
              data-name="v1.0-core-20160216"
              data-skip-pjax="true"
              rel="nofollow">
              <svg aria-hidden="true" class="octicon octicon-check select-menu-item-icon" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M12 5l-8 8-4-4 1.5-1.5L4 10l6.5-6.5z"/></svg>
              <span class="select-menu-item-text css-truncate-target" title="v1.0-core-20160216">
                v1.0-core-20160216
              </span>
            </a>
        </div>

        <div class="select-menu-no-results">Nothing to show</div>
      </div>

    </div>
  </div>
</div>

    <div class="BtnGroup float-right">
      <a href="/KhronosGroup/Vulkan-Docs/find/1.0"
            class="js-pjax-capture-input btn btn-sm BtnGroup-item"
            data-pjax
            data-hotkey="t">
        Find file
      </a>
      <button aria-label="Copy file path to clipboard" class="js-zeroclipboard btn btn-sm BtnGroup-item tooltipped tooltipped-s" data-copied-hint="Copied!" type="button">Copy path</button>
    </div>
    <div class="breadcrumb js-zeroclipboard-target">
      <span class="repo-root js-repo-root"><span class="js-path-segment"><a href="/KhronosGroup/Vulkan-Docs"><span>Vulkan-Docs</span></a></span></span><span class="separator">/</span><span class="js-path-segment"><a href="/KhronosGroup/Vulkan-Docs/tree/1.0/src"><span>src</span></a></span><span class="separator">/</span><span class="js-path-segment"><a href="/KhronosGroup/Vulkan-Docs/tree/1.0/src/ext_loader"><span>ext_loader</span></a></span><span class="separator">/</span><strong class="final-path">vulkan_ext.h</strong>
    </div>
  </div>


  
  <div class="commit-tease">
      <span class="float-right">
        <a class="commit-tease-sha" href="/KhronosGroup/Vulkan-Docs/commit/ca4abe0d34ca8ed8410c65c177f8658da183576d" data-pjax>
          ca4abe0
        </a>
        <relative-time datetime="2017-01-18T04:11:25Z">Jan 18, 2017</relative-time>
      </span>
      <div>
        <img alt="" class="avatar" data-canonical-src="https://2.gravatar.com/avatar/c8ea7ed44063ee5d0b8e58b852785d4c?d=https%3A%2F%2Fassets-cdn.github.com%2Fimages%2Fgravatars%2Fgravatar-user-420.png&amp;r=x&amp;s=140" height="20" src="https://camo.githubusercontent.com/a3426c302404e85e77df1937fc2455201cbe0fad/68747470733a2f2f322e67726176617461722e636f6d2f6176617461722f63386561376564343430363365653564306238653538623835323738356434633f643d68747470732533412532462532466173736574732d63646e2e6769746875622e636f6d253246696d6167657325324667726176617461727325324667726176617461722d757365722d3432302e706e6726723d7826733d313430" width="20" />
        <span class="user-mention">Jon Leech</span>
          <a href="/KhronosGroup/Vulkan-Docs/commit/ca4abe0d34ca8ed8410c65c177f8658da183576d" class="message" data-pjax="true" title="Change log for January 23, 2017 Vulkan 1.0.39 spec update:

  * Bump API patch number and header version number to 39 for this update.

Github Issues:

  * Clarified that only accesses via the specified buffer/image subresource
    ranges are included in the access scopes (public issue 306).
  * Add missing valid usage statements for flink:vkCreateComputePipelines
    and flink:vkCreateGraphicsPipelines (public issue 427).

Internal Issues:

  * Add a Note to the &lt;&lt;invariance,Invariance&gt;&gt; appendix about a difference
    between OpenGL and Vulkan with regards to how primitives derived from
    offsets are handled (internal issue 355).
  * Add the +&lt;&lt;VK_KHR_get_physical_device_properties2&gt;&gt;+,
    +&lt;&lt;VK_KHR_maintenance1&gt;&gt;+, and +&lt;&lt;VK_KHR_shader_draw_parameters&gt;&gt;+
    extensions (internal issue 448).
  * Add the +&lt;&lt;VK_EXT_shader_subgroup_vote&gt;&gt;+ and
    +&lt;&lt;VK_EXT_shader_subgroup_ballot&gt;&gt;+ extensions (internal issue 449).
  * Update the texture level-of-detail equation in the
    &lt;&lt;textures-scale-factor,Scale Factor Operation&gt;&gt; section to better
    approximate the ellipse major and minor axes (internal issue 547).
  * Forbid non-explicitly allowed uses of interface decorations in the
    introduction to the &lt;&lt;interfaces,Shader Interfaces&gt;&gt; chapter (internal
    issue 607).
  * Replace use of MathJax with KaTeX, for improved load-time performance as
    well as avoiding the scrolling-and-scrolling behavior due to MathJax
    asynchronous rendering when loading at an anchor inside the spec. This
    change also requires moving to HTML5 output for the spec instead of
    XHTML, and there is a visible difference in that the chapter navigation
    index is now in a scrollable sidebar instead of at the top of the
    document. We may or may not retain the nav sidebar based on feedback
    (internal issue 613).
  * Improve consistency of markup and formatting in extension appendices
    (internal issue 631).

Other Issues:

  * Add explicit valid usage statements to slink:VkImageCopy requiring that
    the source and destination layer ranges be contained in their respective
    source and destination images.
  * Add valid usage language for swapchain of flink:vkAcquireNextImage. If
    the swapchain has been replaced, then it should not be passed to
    flink:vkAcquireNextImage.
  * Add a valid usage statement to flink:vkCreateImageView, that the image
    must have been created with an appropriate usage bit set.
  * Noted that slink:VkDisplayPresentInfoKHR is a valid extension of
    slink:VkPresentInfoKHR in the &lt;&lt;wsi_swapchain,WSI Swapchain&gt;&gt; section.
  * Update valid usage for flink:vkCmdSetViewport and flink:vkCmdSetScissor
    to account for the multiple viewport feature. If the feature is not
    enabled, the parameters for these functions have required values that
    are defined in the &lt;&lt;features-features-multiViewport,multiple
    viewports&gt;&gt; section of the spec but were not reflected in the valid
    usage text for these functions.
  * Add the +&lt;&lt;VK_EXT_swapchain_colorspace&gt;&gt;+ extension defining common
    color spaces.">Change log for January 23, 2017 Vulkan 1.0.39 spec update:</a>
      </div>

    <div class="commit-tease-contributors">
      <button type="button" class="btn-link muted-link contributors-toggle" data-facebox="#blob_contributors_box">
        <strong>0</strong>
         contributors
      </button>
      
    </div>

    <div id="blob_contributors_box" style="display:none">
      <h2 class="facebox-header" data-facebox-id="facebox-header">Users who have contributed to this file</h2>
      <ul class="facebox-user-list" data-facebox-id="facebox-description">
      </ul>
    </div>
  </div>


  <div class="file">
    <div class="file-header">
  <div class="file-actions">

    <div class="BtnGroup">
      <a href="/KhronosGroup/Vulkan-Docs/raw/1.0/src/ext_loader/vulkan_ext.h" class="btn btn-sm BtnGroup-item" id="raw-url">Raw</a>
        <a href="/KhronosGroup/Vulkan-Docs/blame/1.0/src/ext_loader/vulkan_ext.h" class="btn btn-sm js-update-url-with-hash BtnGroup-item" data-hotkey="b">Blame</a>
      <a href="/KhronosGroup/Vulkan-Docs/commits/1.0/src/ext_loader/vulkan_ext.h" class="btn btn-sm BtnGroup-item" rel="nofollow">History</a>
    </div>

        <a class="btn-octicon tooltipped tooltipped-nw"
           href="github-mac://openRepo/https://github.com/KhronosGroup/Vulkan-Docs?branch=1.0&amp;filepath=src%2Fext_loader%2Fvulkan_ext.h"
           aria-label="Open this file in GitHub Desktop"
           data-ga-click="Repository, open with desktop, type:windows">
            <svg aria-hidden="true" class="octicon octicon-device-desktop" height="16" version="1.1" viewBox="0 0 16 16" width="16"><path fill-rule="evenodd" d="M15 2H1c-.55 0-1 .45-1 1v9c0 .55.45 1 1 1h5.34c-.25.61-.86 1.39-2.34 2h8c-1.48-.61-2.09-1.39-2.34-2H15c.55 0 1-.45 1-1V3c0-.55-.45-1-1-1zm0 9H1V3h14v8z"/></svg>
        </a>

        <!-- '"` --><!-- </textarea></xmp> --></option></form><form accept-charset="UTF-8" action="/KhronosGroup/Vulkan-Docs/edit/1.0/src/ext_loader/vulkan_ext.h" class="inline-form js-update-url-with-hash" method="post"><div style="margin:0;padding:0;display:inline"><input name="utf8" type="hidden" value="&#x2713;" /><input name="authenticity_token" type="hidden" value="r/zi7gSqm5eAGz2i9NcV6Sc+WlWISFv1cdkwcFEFyq0S9MGDu6Khp1kG++qIwpxZawPUbvTkAbQvd6U7oSDD+Q==" /></div>
          <button class="btn-octicon tooltipped tooltipped-nw" type="submit"
            aria-label="Fork this project and edit the file" data-hotkey="e" data-disable-with>
            <svg aria-hidden="true" class="octicon octicon-pencil" height="16" version="1.1" viewBox="0 0 14 16" width="14"><path fill-rule="evenodd" d="M0 12v3h3l8-8-3-3-8 8zm3 2H1v-2h1v1h1v1zm10.3-9.3L12 6 9 3l1.3-1.3a.996.996 0 0 1 1.41 0l1.59 1.59c.39.39.39 1.02 0 1.41z"/></svg>
          </button>
</form>        <!-- '"` --><!-- </textarea></xmp> --></option></form><form accept-charset="UTF-8" action="/KhronosGroup/Vulkan-Docs/delete/1.0/src/ext_loader/vulkan_ext.h" class="inline-form" method="post"><div style="margin:0;padding:0;display:inline"><input name="utf8" type="hidden" value="&#x2713;" /><input name="authenticity_token" type="hidden" value="3xKLAWT6uUAzUkWCdD0r6RMwELNv5hbiH8qKH3ZDxBMy8MpGXhQ1Q5MpbyY/8CYuzI1G9n9fqqNRXCqqMYIs8w==" /></div>
          <button class="btn-octicon btn-octicon-danger tooltipped tooltipped-nw" type="submit"
            aria-label="Fork this project and delete the file" data-disable-with>
            <svg aria-hidden="true" class="octicon octicon-trashcan" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M11 2H9c0-.55-.45-1-1-1H5c-.55 0-1 .45-1 1H2c-.55 0-1 .45-1 1v1c0 .55.45 1 1 1v9c0 .55.45 1 1 1h7c.55 0 1-.45 1-1V5c.55 0 1-.45 1-1V3c0-.55-.45-1-1-1zm-1 12H3V5h1v8h1V5h1v8h1V5h1v8h1V5h1v9zm1-10H2V3h9v1z"/></svg>
          </button>
</form>  </div>

  <div class="file-info">
      64 lines (55 sloc)
      <span class="file-info-divider"></span>
    2.03 KB
  </div>
</div>

    

  <div itemprop="text" class="blob-wrapper data type-c">
      <table class="highlight tab-size js-file-line-container" data-tab-size="8">
      <tr>
        <td id="L1" class="blob-num js-line-number" data-line-number="1"></td>
        <td id="LC1" class="blob-code blob-code-inner js-file-line">#<span class="pl-k">ifndef</span> VULKAN_EXT_H</td>
      </tr>
      <tr>
        <td id="L2" class="blob-num js-line-number" data-line-number="2"></td>
        <td id="LC2" class="blob-code blob-code-inner js-file-line">#<span class="pl-k">define</span> <span class="pl-en">VULKAN_EXT_H</span></td>
      </tr>
      <tr>
        <td id="L3" class="blob-num js-line-number" data-line-number="3"></td>
        <td id="LC3" class="blob-code blob-code-inner js-file-line">
</td>
      </tr>
      <tr>
        <td id="L4" class="blob-num js-line-number" data-line-number="4"></td>
        <td id="LC4" class="blob-code blob-code-inner js-file-line">#<span class="pl-k">ifdef</span> __cplusplus</td>
      </tr>
      <tr>
        <td id="L5" class="blob-num js-line-number" data-line-number="5"></td>
        <td id="LC5" class="blob-code blob-code-inner js-file-line"><span class="pl-k">extern</span> <span class="pl-s"><span class="pl-pds">&quot;</span>C<span class="pl-pds">&quot;</span></span> {</td>
      </tr>
      <tr>
        <td id="L6" class="blob-num js-line-number" data-line-number="6"></td>
        <td id="LC6" class="blob-code blob-code-inner js-file-line">#endif</td>
      </tr>
      <tr>
        <td id="L7" class="blob-num js-line-number" data-line-number="7"></td>
        <td id="LC7" class="blob-code blob-code-inner js-file-line"><span class="pl-c"><span class="pl-c">/*</span></span></td>
      </tr>
      <tr>
        <td id="L8" class="blob-num js-line-number" data-line-number="8"></td>
        <td id="LC8" class="blob-code blob-code-inner js-file-line"><span class="pl-c">** Copyright (c) 2015-2017 The Khronos Group Inc.</span></td>
      </tr>
      <tr>
        <td id="L9" class="blob-num js-line-number" data-line-number="9"></td>
        <td id="LC9" class="blob-code blob-code-inner js-file-line"><span class="pl-c">**</span></td>
      </tr>
      <tr>
        <td id="L10" class="blob-num js-line-number" data-line-number="10"></td>
        <td id="LC10" class="blob-code blob-code-inner js-file-line"><span class="pl-c">** Licensed under the Apache License, Version 2.0 (the &quot;License&quot;);</span></td>
      </tr>
      <tr>
        <td id="L11" class="blob-num js-line-number" data-line-number="11"></td>
        <td id="LC11" class="blob-code blob-code-inner js-file-line"><span class="pl-c">** you may not use this file except in compliance with the License.</span></td>
      </tr>
      <tr>
        <td id="L12" class="blob-num js-line-number" data-line-number="12"></td>
        <td id="LC12" class="blob-code blob-code-inner js-file-line"><span class="pl-c">** You may obtain a copy of the License at</span></td>
      </tr>
      <tr>
        <td id="L13" class="blob-num js-line-number" data-line-number="13"></td>
        <td id="LC13" class="blob-code blob-code-inner js-file-line"><span class="pl-c">**</span></td>
      </tr>
      <tr>
        <td id="L14" class="blob-num js-line-number" data-line-number="14"></td>
        <td id="LC14" class="blob-code blob-code-inner js-file-line"><span class="pl-c">**     http://www.apache.org/licenses/LICENSE-2.0</span></td>
      </tr>
      <tr>
        <td id="L15" class="blob-num js-line-number" data-line-number="15"></td>
        <td id="LC15" class="blob-code blob-code-inner js-file-line"><span class="pl-c">**</span></td>
      </tr>
      <tr>
        <td id="L16" class="blob-num js-line-number" data-line-number="16"></td>
        <td id="LC16" class="blob-code blob-code-inner js-file-line"><span class="pl-c">** Unless required by applicable law or agreed to in writing, software</span></td>
      </tr>
      <tr>
        <td id="L17" class="blob-num js-line-number" data-line-number="17"></td>
        <td id="LC17" class="blob-code blob-code-inner js-file-line"><span class="pl-c">** distributed under the License is distributed on an &quot;AS IS&quot; BASIS,</span></td>
      </tr>
      <tr>
        <td id="L18" class="blob-num js-line-number" data-line-number="18"></td>
        <td id="LC18" class="blob-code blob-code-inner js-file-line"><span class="pl-c">** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.</span></td>
      </tr>
      <tr>
        <td id="L19" class="blob-num js-line-number" data-line-number="19"></td>
        <td id="LC19" class="blob-code blob-code-inner js-file-line"><span class="pl-c">** See the License for the specific language governing permissions and</span></td>
      </tr>
      <tr>
        <td id="L20" class="blob-num js-line-number" data-line-number="20"></td>
        <td id="LC20" class="blob-code blob-code-inner js-file-line"><span class="pl-c">** limitations under the License.</span></td>
      </tr>
      <tr>
        <td id="L21" class="blob-num js-line-number" data-line-number="21"></td>
        <td id="LC21" class="blob-code blob-code-inner js-file-line"><span class="pl-c"><span class="pl-c">*/</span></span></td>
      </tr>
      <tr>
        <td id="L22" class="blob-num js-line-number" data-line-number="22"></td>
        <td id="LC22" class="blob-code blob-code-inner js-file-line">
</td>
      </tr>
      <tr>
        <td id="L23" class="blob-num js-line-number" data-line-number="23"></td>
        <td id="LC23" class="blob-code blob-code-inner js-file-line"><span class="pl-c"><span class="pl-c">/*</span></span></td>
      </tr>
      <tr>
        <td id="L24" class="blob-num js-line-number" data-line-number="24"></td>
        <td id="LC24" class="blob-code blob-code-inner js-file-line"><span class="pl-c">** This header is generated from the Khronos Vulkan XML API Registry.</span></td>
      </tr>
      <tr>
        <td id="L25" class="blob-num js-line-number" data-line-number="25"></td>
        <td id="LC25" class="blob-code blob-code-inner js-file-line"><span class="pl-c">**</span></td>
      </tr>
      <tr>
        <td id="L26" class="blob-num js-line-number" data-line-number="26"></td>
        <td id="LC26" class="blob-code blob-code-inner js-file-line"><span class="pl-c"><span class="pl-c">*/</span></span></td>
      </tr>
      <tr>
        <td id="L27" class="blob-num js-line-number" data-line-number="27"></td>
        <td id="LC27" class="blob-code blob-code-inner js-file-line">
</td>
      </tr>
      <tr>
        <td id="L28" class="blob-num js-line-number" data-line-number="28"></td>
        <td id="LC28" class="blob-code blob-code-inner js-file-line">
</td>
      </tr>
      <tr>
        <td id="L29" class="blob-num js-line-number" data-line-number="29"></td>
        <td id="LC29" class="blob-code blob-code-inner js-file-line"><span class="pl-c"><span class="pl-c">/*</span></span></td>
      </tr>
      <tr>
        <td id="L30" class="blob-num js-line-number" data-line-number="30"></td>
        <td id="LC30" class="blob-code blob-code-inner js-file-line"><span class="pl-c">** This is a simple extension loader which provides the implementations for the</span></td>
      </tr>
      <tr>
        <td id="L31" class="blob-num js-line-number" data-line-number="31"></td>
        <td id="LC31" class="blob-code blob-code-inner js-file-line"><span class="pl-c">** extension prototypes declared in vulkan header. It supports loading extensions either</span></td>
      </tr>
      <tr>
        <td id="L32" class="blob-num js-line-number" data-line-number="32"></td>
        <td id="LC32" class="blob-code blob-code-inner js-file-line"><span class="pl-c">** for a single instance or a single device. Multiple instances are not yet supported.</span></td>
      </tr>
      <tr>
        <td id="L33" class="blob-num js-line-number" data-line-number="33"></td>
        <td id="LC33" class="blob-code blob-code-inner js-file-line"><span class="pl-c">**</span></td>
      </tr>
      <tr>
        <td id="L34" class="blob-num js-line-number" data-line-number="34"></td>
        <td id="LC34" class="blob-code blob-code-inner js-file-line"><span class="pl-c">** To use the loader add vulkan_ext.c to your solution and include &lt;vulkan/vulkan_ext.h&gt;.</span></td>
      </tr>
      <tr>
        <td id="L35" class="blob-num js-line-number" data-line-number="35"></td>
        <td id="LC35" class="blob-code blob-code-inner js-file-line"><span class="pl-c">**</span></td>
      </tr>
      <tr>
        <td id="L36" class="blob-num js-line-number" data-line-number="36"></td>
        <td id="LC36" class="blob-code blob-code-inner js-file-line"><span class="pl-c">** If your application is using a single instance, but multiple devices callParam</span></td>
      </tr>
      <tr>
        <td id="L37" class="blob-num js-line-number" data-line-number="37"></td>
        <td id="LC37" class="blob-code blob-code-inner js-file-line"><span class="pl-c">**</span></td>
      </tr>
      <tr>
        <td id="L38" class="blob-num js-line-number" data-line-number="38"></td>
        <td id="LC38" class="blob-code blob-code-inner js-file-line"><span class="pl-c">** vkExtInitInstance(instance);</span></td>
      </tr>
      <tr>
        <td id="L39" class="blob-num js-line-number" data-line-number="39"></td>
        <td id="LC39" class="blob-code blob-code-inner js-file-line"><span class="pl-c">**</span></td>
      </tr>
      <tr>
        <td id="L40" class="blob-num js-line-number" data-line-number="40"></td>
        <td id="LC40" class="blob-code blob-code-inner js-file-line"><span class="pl-c">** after initializing the instance. This way the extension loader will use the loaders</span></td>
      </tr>
      <tr>
        <td id="L41" class="blob-num js-line-number" data-line-number="41"></td>
        <td id="LC41" class="blob-code blob-code-inner js-file-line"><span class="pl-c">** trampoline functions to call the correct driver for each call. This method is safe</span></td>
      </tr>
      <tr>
        <td id="L42" class="blob-num js-line-number" data-line-number="42"></td>
        <td id="LC42" class="blob-code blob-code-inner js-file-line"><span class="pl-c">** if your application might use more than one device at the cost of one additional</span></td>
      </tr>
      <tr>
        <td id="L43" class="blob-num js-line-number" data-line-number="43"></td>
        <td id="LC43" class="blob-code blob-code-inner js-file-line"><span class="pl-c">** indirection, the dispatch table of each dispatchable object.</span></td>
      </tr>
      <tr>
        <td id="L44" class="blob-num js-line-number" data-line-number="44"></td>
        <td id="LC44" class="blob-code blob-code-inner js-file-line"><span class="pl-c">**</span></td>
      </tr>
      <tr>
        <td id="L45" class="blob-num js-line-number" data-line-number="45"></td>
        <td id="LC45" class="blob-code blob-code-inner js-file-line"><span class="pl-c">** If your application uses only a single device it&#39;s better to use</span></td>
      </tr>
      <tr>
        <td id="L46" class="blob-num js-line-number" data-line-number="46"></td>
        <td id="LC46" class="blob-code blob-code-inner js-file-line"><span class="pl-c">**</span></td>
      </tr>
      <tr>
        <td id="L47" class="blob-num js-line-number" data-line-number="47"></td>
        <td id="LC47" class="blob-code blob-code-inner js-file-line"><span class="pl-c">** vkExtInitDevice(device);</span></td>
      </tr>
      <tr>
        <td id="L48" class="blob-num js-line-number" data-line-number="48"></td>
        <td id="LC48" class="blob-code blob-code-inner js-file-line"><span class="pl-c">**</span></td>
      </tr>
      <tr>
        <td id="L49" class="blob-num js-line-number" data-line-number="49"></td>
        <td id="LC49" class="blob-code blob-code-inner js-file-line"><span class="pl-c">** once the device has been initialized. This will resolve the function pointers</span></td>
      </tr>
      <tr>
        <td id="L50" class="blob-num js-line-number" data-line-number="50"></td>
        <td id="LC50" class="blob-code blob-code-inner js-file-line"><span class="pl-c">** upfront and thus removes one indirection for each call into the driver. This *can*</span></td>
      </tr>
      <tr>
        <td id="L51" class="blob-num js-line-number" data-line-number="51"></td>
        <td id="LC51" class="blob-code blob-code-inner js-file-line"><span class="pl-c">** result in slightly more performance for calling overhead limited cases.</span></td>
      </tr>
      <tr>
        <td id="L52" class="blob-num js-line-number" data-line-number="52"></td>
        <td id="LC52" class="blob-code blob-code-inner js-file-line"><span class="pl-c"><span class="pl-c">*/</span></span></td>
      </tr>
      <tr>
        <td id="L53" class="blob-num js-line-number" data-line-number="53"></td>
        <td id="LC53" class="blob-code blob-code-inner js-file-line">
</td>
      </tr>
      <tr>
        <td id="L54" class="blob-num js-line-number" data-line-number="54"></td>
        <td id="LC54" class="blob-code blob-code-inner js-file-line">#<span class="pl-k">include</span> <span class="pl-s"><span class="pl-pds">&lt;</span>vulkan/vulkan.h<span class="pl-pds">&gt;</span></span></td>
      </tr>
      <tr>
        <td id="L55" class="blob-num js-line-number" data-line-number="55"></td>
        <td id="LC55" class="blob-code blob-code-inner js-file-line">
</td>
      </tr>
      <tr>
        <td id="L56" class="blob-num js-line-number" data-line-number="56"></td>
        <td id="LC56" class="blob-code blob-code-inner js-file-line"><span class="pl-k">void</span> <span class="pl-smi">vkExtInitInstance</span>(VkInstance instance);</td>
      </tr>
      <tr>
        <td id="L57" class="blob-num js-line-number" data-line-number="57"></td>
        <td id="LC57" class="blob-code blob-code-inner js-file-line"><span class="pl-k">void</span> <span class="pl-smi">vkExtInitDevice</span>(VkDevice device);</td>
      </tr>
      <tr>
        <td id="L58" class="blob-num js-line-number" data-line-number="58"></td>
        <td id="LC58" class="blob-code blob-code-inner js-file-line">
</td>
      </tr>
      <tr>
        <td id="L59" class="blob-num js-line-number" data-line-number="59"></td>
        <td id="LC59" class="blob-code blob-code-inner js-file-line">#<span class="pl-k">ifdef</span> __cplusplus</td>
      </tr>
      <tr>
        <td id="L60" class="blob-num js-line-number" data-line-number="60"></td>
        <td id="LC60" class="blob-code blob-code-inner js-file-line">}</td>
      </tr>
      <tr>
        <td id="L61" class="blob-num js-line-number" data-line-number="61"></td>
        <td id="LC61" class="blob-code blob-code-inner js-file-line">#<span class="pl-k">endif</span></td>
      </tr>
      <tr>
        <td id="L62" class="blob-num js-line-number" data-line-number="62"></td>
        <td id="LC62" class="blob-code blob-code-inner js-file-line">
</td>
      </tr>
      <tr>
        <td id="L63" class="blob-num js-line-number" data-line-number="63"></td>
        <td id="LC63" class="blob-code blob-code-inner js-file-line">#endif</td>
      </tr>
</table>

  <div class="BlobToolbar position-absolute js-file-line-actions dropdown js-menu-container js-select-menu d-none" aria-hidden="true">
    <button class="btn-octicon ml-0 px-2 p-0 bg-white border border-gray-dark rounded-1 dropdown-toggle js-menu-target" id="js-file-line-action-button" type="button" aria-expanded="false" aria-haspopup="true" aria-label="Inline file action toolbar" aria-controls="inline-file-actions">
      <svg aria-hidden="true" class="octicon" height="16" version="1.1" viewBox="0 0 13 4" width="14">
        <g stroke="none" stroke-width="1" fill-rule="evenodd">
            <g transform="translate(-1.000000, -6.000000)">
                <path d="M2.5,9.5 C1.67157288,9.5 1,8.82842712 1,8 C1,7.17157288 1.67157288,6.5 2.5,6.5 C3.32842712,6.5 4,7.17157288 4,8 C4,8.82842712 3.32842712,9.5 2.5,9.5 Z M7.5,9.5 C6.67157288,9.5 6,8.82842712 6,8 C6,7.17157288 6.67157288,6.5 7.5,6.5 C8.32842712,6.5 9,7.17157288 9,8 C9,8.82842712 8.32842712,9.5 7.5,9.5 Z M12.5,9.5 C11.6715729,9.5 11,8.82842712 11,8 C11,7.17157288 11.6715729,6.5 12.5,6.5 C13.3284271,6.5 14,7.17157288 14,8 C14,8.82842712 13.3284271,9.5 12.5,9.5 Z"></path>
            </g>
        </g>
      </svg>
    </button>
    <div class="dropdown-menu-content js-menu-content" id="inline-file-actions">
      <ul class="BlobToolbar-dropdown dropdown-menu dropdown-menu-se mt-2">
        <li><a class="js-zeroclipboard dropdown-item" style="cursor:pointer;" id="js-copy-lines" data-original-text="Copy lines">Copy lines</a></li>
        <li><a class="js-zeroclipboard dropdown-item" id= "js-copy-permalink" style="cursor:pointer;" data-original-text="Copy permalink">Copy permalink</a></li>
        <li><a href="/KhronosGroup/Vulkan-Docs/blame/6aa7d7bd2c447a3f10433ccd3b38a82e2a587eac/src/ext_loader/vulkan_ext.h" class="dropdown-item js-update-url-with-hash" id="js-view-git-blame">View git blame</a></li>
          <li><a href="/KhronosGroup/Vulkan-Docs/issues/new" class="dropdown-item" id="js-new-issue">Open new issue</a></li>
      </ul>
    </div>
  </div>

  </div>

  </div>

  <button type="button" data-facebox="#jump-to-line" data-facebox-class="linejump" data-hotkey="l" class="d-none">Jump to Line</button>
  <div id="jump-to-line" style="display:none">
    <!-- '"` --><!-- </textarea></xmp> --></option></form><form accept-charset="UTF-8" action="" class="js-jump-to-line-form" method="get"><div style="margin:0;padding:0;display:inline"><input name="utf8" type="hidden" value="&#x2713;" /></div>
      <input class="form-control linejump-input js-jump-to-line-field" type="text" placeholder="Jump to line&hellip;" aria-label="Jump to line" autofocus>
      <button type="submit" class="btn">Go</button>
</form>  </div>

  </div>
  <div class="modal-backdrop js-touch-events"></div>
</div>

    </div>
  </div>

  </div>

      
<div class="footer container-lg px-3" role="contentinfo">
  <div class="position-relative d-flex flex-justify-between py-6 mt-6 f6 text-gray border-top border-gray-light ">
    <ul class="list-style-none d-flex flex-wrap ">
      <li class="mr-3">&copy; 2017 <span title="0.28041s from unicorn-3766587846-6l35l">GitHub</span>, Inc.</li>
        <li class="mr-3"><a href="https://github.com/site/terms" data-ga-click="Footer, go to terms, text:terms">Terms</a></li>
        <li class="mr-3"><a href="https://github.com/site/privacy" data-ga-click="Footer, go to privacy, text:privacy">Privacy</a></li>
        <li class="mr-3"><a href="https://github.com/security" data-ga-click="Footer, go to security, text:security">Security</a></li>
        <li class="mr-3"><a href="https://status.github.com/" data-ga-click="Footer, go to status, text:status">Status</a></li>
        <li><a href="https://help.github.com" data-ga-click="Footer, go to help, text:help">Help</a></li>
    </ul>

    <a href="https://github.com" aria-label="Homepage" class="footer-octicon" title="GitHub">
      <svg aria-hidden="true" class="octicon octicon-mark-github" height="24" version="1.1" viewBox="0 0 16 16" width="24"><path fill-rule="evenodd" d="M8 0C3.58 0 0 3.58 0 8c0 3.54 2.29 6.53 5.47 7.59.4.07.55-.17.55-.38 0-.19-.01-.82-.01-1.49-2.01.37-2.53-.49-2.69-.94-.09-.23-.48-.94-.82-1.13-.28-.15-.68-.52-.01-.53.63-.01 1.08.58 1.23.82.72 1.21 1.87.87 2.33.66.07-.52.28-.87.51-1.07-1.78-.2-3.64-.89-3.64-3.95 0-.87.31-1.59.82-2.15-.08-.2-.36-1.02.08-2.12 0 0 .67-.21 2.2.82.64-.18 1.32-.27 2-.27.68 0 1.36.09 2 .27 1.53-1.04 2.2-.82 2.2-.82.44 1.1.16 1.92.08 2.12.51.56.82 1.27.82 2.15 0 3.07-1.87 3.75-3.65 3.95.29.25.54.73.54 1.48 0 1.07-.01 1.93-.01 2.2 0 .21.15.46.55.38A8.013 8.013 0 0 0 16 8c0-4.42-3.58-8-8-8z"/></svg>
</a>
    <ul class="list-style-none d-flex flex-wrap ">
        <li class="mr-3"><a href="https://github.com/contact" data-ga-click="Footer, go to contact, text:contact">Contact GitHub</a></li>
      <li class="mr-3"><a href="https://developer.github.com" data-ga-click="Footer, go to api, text:api">API</a></li>
      <li class="mr-3"><a href="https://training.github.com" data-ga-click="Footer, go to training, text:training">Training</a></li>
      <li class="mr-3"><a href="https://shop.github.com" data-ga-click="Footer, go to shop, text:shop">Shop</a></li>
        <li class="mr-3"><a href="https://github.com/blog" data-ga-click="Footer, go to blog, text:blog">Blog</a></li>
        <li><a href="https://github.com/about" data-ga-click="Footer, go to about, text:about">About</a></li>

    </ul>
  </div>
</div>



  <div id="ajax-error-message" class="ajax-error-message flash flash-error">
    <svg aria-hidden="true" class="octicon octicon-alert" height="16" version="1.1" viewBox="0 0 16 16" width="16"><path fill-rule="evenodd" d="M8.865 1.52c-.18-.31-.51-.5-.87-.5s-.69.19-.87.5L.275 13.5c-.18.31-.18.69 0 1 .19.31.52.5.87.5h13.7c.36 0 .69-.19.86-.5.17-.31.18-.69.01-1L8.865 1.52zM8.995 13h-2v-2h2v2zm0-3h-2V6h2v4z"/></svg>
    <button type="button" class="flash-close js-flash-close js-ajax-error-dismiss" aria-label="Dismiss error">
      <svg aria-hidden="true" class="octicon octicon-x" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M7.48 8l3.75 3.75-1.48 1.48L6 9.48l-3.75 3.75-1.48-1.48L4.52 8 .77 4.25l1.48-1.48L6 6.52l3.75-3.75 1.48 1.48z"/></svg>
    </button>
    You can't perform that action at this time.
  </div>


    
    <script crossorigin="anonymous" integrity="sha256-0/jjywXESVr8eMN68y6Hf5nUfLBqgVjvr6Kuv1VnyDA=" src="https://assets-cdn.github.com/assets/frameworks-d3f8e3cb05c4495afc78c37af32e877f99d47cb06a8158efafa2aebf5567c830.js"></script>
    
    <script async="async" crossorigin="anonymous" integrity="sha256-imYlhu1Qzl0SLt7A7KTC2wUrY1iKs/wZK0GPJh0tC1M=" src="https://assets-cdn.github.com/assets/github-8a662586ed50ce5d122edec0eca4c2db052b63588ab3fc192b418f261d2d0b53.js"></script>
    
    
    
    
  <div class="js-stale-session-flash stale-session-flash flash flash-warn flash-banner d-none">
    <svg aria-hidden="true" class="octicon octicon-alert" height="16" version="1.1" viewBox="0 0 16 16" width="16"><path fill-rule="evenodd" d="M8.865 1.52c-.18-.31-.51-.5-.87-.5s-.69.19-.87.5L.275 13.5c-.18.31-.18.69 0 1 .19.31.52.5.87.5h13.7c.36 0 .69-.19.86-.5.17-.31.18-.69.01-1L8.865 1.52zM8.995 13h-2v-2h2v2zm0-3h-2V6h2v4z"/></svg>
    <span class="signed-in-tab-flash">You signed in with another tab or window. <a href="">Reload</a> to refresh your session.</span>
    <span class="signed-out-tab-flash">You signed out in another tab or window. <a href="">Reload</a> to refresh your session.</span>
  </div>
  <div class="facebox" id="facebox" style="display:none;">
  <div class="facebox-popup">
    <div class="facebox-content" role="dialog" aria-labelledby="facebox-header" aria-describedby="facebox-description">
    </div>
    <button type="button" class="facebox-close js-facebox-close" aria-label="Close modal">
      <svg aria-hidden="true" class="octicon octicon-x" height="16" version="1.1" viewBox="0 0 12 16" width="12"><path fill-rule="evenodd" d="M7.48 8l3.75 3.75-1.48 1.48L6 9.48l-3.75 3.75-1.48-1.48L4.52 8 .77 4.25l1.48-1.48L6 6.52l3.75-3.75 1.48 1.48z"/></svg>
    </button>
  </div>
</div>


  </body>
</html>

